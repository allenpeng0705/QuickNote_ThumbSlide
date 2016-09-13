#include <limits.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <string.h>

//#define FILTER_DEBUG
//#define FILTER_LOGGER

#include "q_filter_private.h"
#include "base.h"
#include "q_io.h"
#include "q_malloc.h"
#include "utf8_string.h"

#ifdef FILTER_LOGGER
#include "q_logger.h"
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SHAPE_SIGMA 0.2f

#define LOCATION_SIGMA 0.2f

/*
 * The maximum number of characters permissable on a
 * separate line in a UTF-8 encoded lexicon.
 *
 * This macro must be an integer > 0
 */
#define MAX_CHARS_PER_LEXICON_LINE 100

/*
 * The maximum number of bytes permissable on a
 * separate line in a UTF-8 encoded lexicon.
 *
 * This macro must be an integer > 0
 * This macro depends on MAX_CHARS_PER_LEXICON_LINE > 0
 */
#define MAX_BYTES_PER_LEXICON_LINE (MAX_CHARS_PER_LEXICON_LINE * 4)

/*
 * The maximum number of recognition results the recognizer
 * will output.
 */
#define MAX_RESULTS 10

/*
 * Maximum number of characters in an individual string that can
 * be recognized by the recognizer. Excluding the null-terminator.
 *
 * This macro must be an integer > 0
 */
#define MAX_CHARS_PER_STRING 50

/*
 * Maximum number of bytes in an individual string that can
 * be recognized by the recognizer. Excluding the null-terminator.
 *
 * This macro must be an integer > 0
 * This macro depends on MAX_CHARS_PER_STRING > 0
 */
#define MAX_BYTES_PER_STRING (MAX_CHARS_PER_STRING * 4)

/*
 * Maximum number of characters in a string that contains a recognized
 * word. Including the null-terminator.
 *
 * This macro depends on MAX_BYTES_PER_STRING > -1
 */
#define MAX_BYTES_PER_RESULT (MAX_BYTES_PER_STRING + 1)

/*
 * Max children at a certain encoding node.
 *
 * This number limits the number of characters that can be
 * encoded at a specific (x,y) encoding position.
 */
#define MAX_ENCODING_NODE_CHILD_COUNT 64



/*
 * The heap size. This parameter affects the scope of the final search space
 * in the pattern recognition process. A too large heap size results in too
 * high latency. A too low heap size results in a high probability that the
 * final search will miss the best candidate.
 */
#define HEAP_CAPACITY 30

/*
 * The number of sample points the equidistant resampler will resample
 * a pattern into when performing recognitio at the final stage.
 */
//#define N_SAMPLE_POINTS 100
#define MAX_N_SAMPLE_POINTS 100

/*
 * Maximum number of segments a raw (non-processed) pattern can have.
 * The number of segments in a pattern is (N - 1), where N is the number
 * of sample points in the pattern.
 *
 */
#define MAX_SEGMENTS MAX_N_SAMPLE_POINTS

/*
 * The number of sample points the equidistant resampler will resample
 * a pattern into when pruning.
 */
//#define N_PRUNING_SAMPLE_POINTSS 40
#define MAX_N_PRUNING_SAMPLE_POINTS MAX_N_SAMPLE_POINTS

/*
 * Patterns originated from the N closest keys around
 * the first point in the input signal are searched.
 * All other patterns are immediately discarded.
 */
//#define N_START_POINT_SEARCHED_KEYS 5

/*
 * Patterns originated from the N closest keys around
 * the end point in the input signal are searched.
 * All other patterns are immediately discarded.
 */
//#define N_END_POINT_SEARCHED_KEYS 5

/*
 * This is a threshold used at the pruning stage.
 * When (at the pruning stage) a WordPoint is compared
 * against a corresponding point in a word template,
 * if the point-to-point distance exceeds the above
 * number multiplied with the width of an alphabetic
 * key on the keyboard, the word patterns is immediately
 * discarded from further consideration in the matching
 * process.
 */
#define KEY_WIDTH_PT_TO_PT_THRESHOLD 4



#define CORNER_ANGLE_THRESHOLD	0.9397		// cos20 = 0.9797
#define MAX_N_NEIGHBORS			6


/*
 * Encoding:
 *
 * The recognizer stores all strings that are recognizable for provided keyboard layout in
 * a special format.
 *
 * A "pattern" is a sequence of (x,y) points that corresponds to a string. An "encoded pattern"
 * is a pattern encoded in the special format. Encoding works as follows:
 *
 * Each letter key in a keyboard layout has three unique attributes: the letter, the x coordinate,
 * and the y-coordinate. All variations of these attributes are stored three sequences, one for
 * letters (c_comp), one for x-coordinates (x_comp) and one for y-coordinates (y_comp).
 *
 * Each letter key is assigned a unique index number that indexes that letter key's letter, x and
 * y-coordinates in c_comp, x_comp and y_comp. The index numbers are limited to the range [0,255],
 * which means each character in a string, as well as its geometric center point of the its corresponding
 * letter key, can be specified in one byte.
 *
 * Storage:
 *
 * The storage area is a contiguous sequence of unsigned bytes in the range [0,255]. The instance variable
 * "storage" points to the first byte in the storage area. The storage consists of encoded patterns prefixed
 * with a 2-byte pattern header (length and other information of the pattern).The instance variable "storage"
 * points to the first such pattern in the byte sequence.
 *
 * Indexing:
 *
 * The instance variable "indices" points to a sequence of Index objects. Each Index object
 *
 * Creating storage and indices:
 *
 * 
 *
 */

/*
 * Pattern specification in storage:
 *
 * Byte 1: The length of the pattern (number of points/letters).
 * Byte 2: Data segment for the pattern.
 * Byte 3-n: Each byte is an encoded point/letter.
 * 
 * Data segment specification:
 *
 * Bit 0-6: Relative rank of the pattern (frequent patterns (``words'') rank
 *			higher than less frequent).
 * Bit 7: Reserved.
 * Bit 8: If ON, the word is in the active vocabulary, otherwise it is passive.
 */

/*
 * The size of the header (or prefix) preceding the
 * encoded pattern specification (in storage) in bytes.
 */
#define PATTERN_HEADER_SIZE 2

#define DS_ACTIVE_MASK ((uint8)(((uint8)1) << 6))
#define DS_RESERVED_MASK ((uint8)(((uint8)1) << 7))
#define DS_RANK_MIN 0
#define DS_RANK_MAX 63
#define ECHAR_ESCAPED_MASK ((uint8)(((uint8)1) << 7))

/*
 * This parameter controls the ratio in [0,1] of the
 * total set of recognizable strings that are considered
 * active when the internal storage is created.
 */
#define ACTIVE_PARTITION_PERCENTAGE 0.1

/*
 * For detail, see reference of the function: 
 * int16 compare_distance(int16 x1, int16 y1, int16 x2, int16 y2, int16 key_width, int16 key_height)
 */
#define compare_distance(x1, y1, x2, y2, key_width, key_height) (min(abs(x1 - x2) / (key_width / 2) + abs(y1 - y2) / (key_height / 2), 3))


/*
 * This function prototypes takea a data buffer as an argument
 *
 * *in_data_buffer - the data buffer for the function to read
 * *out_line_buffer - the outbuffer to fill with a UTF-8 encoded NULL-terminated
 * text line with all newline characters removed
 * 
 * The return value must be *out_line_buffer or NULL if there are no more text
 * lines to retrieve.
 */
typedef char *(*data_buffer_getline)(void *in_data_buffer, char *out_line_buffer);

typedef enum InternalFilterResult InternalFilterResult;
typedef enum RankMethod RankMethod;

typedef struct Parameters Parameters;
typedef struct MemoryDataBuffer MemoryDataBuffer;
typedef struct EncodingNode EncodingNode;
typedef struct EncodingTable EncodingTable;
typedef struct Bounds Bounds;
typedef struct Result Result;
typedef struct Neighbor Neighbor;
typedef struct Index Index;
typedef struct Bucket Bucket;
typedef struct Heap Heap;
typedef struct HeapNode HeapNode;
typedef struct EdgeMap EdgeMap;
typedef struct KeyTraceMatch KeyTraceMatch;
typedef struct EdgeKeyMap EdgeKeyMap;
typedef struct IndexIgnoreList IndexIgnoreList;

enum InternalFilterResult {
	FILTER_SUCCESS,
	ERROR_UNENCODABLE_STRING,
	MEMORY_ALLOC_FAILURE,
	ERROR_BUFFER_READ_ERROR,
	ERROR_BUFFER_TOO_SMALL
};

enum RankMethod {
	RANK_ACTIVEPASSIVE_PARTITIONED,
	RANK_ACTIVEPASSIVE_SHIFTED
};

/* Holds dynamically adjustable parameters */
struct Parameters {
	uint8 n_start_point_searched_keys;
	uint8 n_end_point_searched_keys;
	uint8 n_pruning_sample_points;
	uint8 n_sample_points;
	int16 key_width;
	int16 key_height;
};

/*
 * A memory data buffer encapsulates all the necessary information to read
 * text lines from a char block from memory as a stream.
 */
struct MemoryDataBuffer {
	/* This fixed WordPointer always WordPoints at the start of the memory buffer */
	char *block_begin;
	/* This pointer moves from the beginning and forwards throughout the memory buffer */
	char *pointer;
	/* The current count of the number of lines that have been read from the memory buffer */
	int32 current_line_count;
	/* The fixed number of lines present in the memory buffer */
	int32 total_line_count;
};

/*
 * An encoding node holds one Unicode character (code WordPoint) and a link
 * that may WordPoint to another encoding node.
 */
struct EncodingNode {
	/* Encoded Unicode character represented by this node */
	int32 ec;
	/* pointer to next child, or NULL if there is no next child */
	EncodingNode *next;
};

/*
 * An encoding table holds a series of encoding vertices. An encoding
 * vertex is a natural number (an index) that indexes *encoding_nodes,
 * *x_components, *y_components.
 *
 * (x,y) pairs must be unique. This means that there must only exist
 * one (encoding vertex) that indexes the same (x,y) coordinate.
 *
 * An encoding vertex corresponds to exactly one (x,y) pair (found in
 * the *x_components and *y_components) and at least one encoding node.
 * Each encoding node represents one Unicode character (code point). This
 * means each (x,y) pair corresponds to at least one Unicode character,
 * but possiblt several.
 */
struct EncodingTable {
	/* Number of encoding vertices */
	uint8 n;
	/* Encoding nodes indexed from 0 to n */
	EncodingNode *encoding_nodes;
	/* X coordinates indexed from 0 to n */
	int16 *x_components;
	/* Y coordinates indexed from 0 to n */
	int16 *y_components;
};

/*
 * Defines a 2D rectangular boundary.
 */
struct Bounds {
	int16 min_x;
	int16 min_y;
	int16 max_x;
	int16 max_y;
};

/*
 * Holds the pair (pattern,value) for primarily
 * temporary sorting purposes.
 */
struct Result {
	/* pointer to the pattern in the storage area */
	uint8* pat_ptr;
	/* Value (often distance, relative to a volatile query) */
	float32 value;
};

/*
 * Holds the pair (index,distance) for primarily
 * temporary sorting purposes.
 */
struct Neighbor {
	/* Encoding index */
	uint8 index;
	/* Distance (relative to a volatile query) */
	float32 distance;
};

/*
 * Encapsulates an indexed block of patterns in storage.
 */
struct Index {
	/* pointer to the first pattern indexed by this index in the storage area */
	uint8 *ptr;
	/* The number of patterns indexed by this index */
	int32 len;
	/* The encoded edge crossings of the patterns */
	uint8 *pecs;
	/* The horizontal directional constraints of the patterns */
	uint8 *xdts;
	/* The vertical directional constraints of the patterns */
	uint8 *ydts;
	/* The horizontal directional constraints around the origin of the patterns */
	uint8 *x0dts;
	/* The vertical directional constraints around the origin of the patterns */
	uint8 *y0dts;
	/* The spatial lengths of the patterns */
	int16 *lengths;
};

/*
 * Holds the pair (count,size) for primarily temporary purposes.
 */
struct Bucket {
	/* Number of patterns in bucket */
	int32 n;
	/* Size of bucket in bytes */
	int32 size;
};

/*
 * Encapsulats a priority heap based on a binary tree.
 */
struct Heap {
	/* An array of nodes arranged to keep the priority */
	HeapNode *nodes;
	/* The number of nodes currently inserted into the heap */
	int32 n;
	/* The maximum number of nodes that can be inserted into the heap */
	int32 capacity;
};

/*
 * Encapsulates a single node in the heap.
 */
struct HeapNode {
	/* The distance (value) contained by this node */
	float32 distance;
	/* A pointer to where the pattern associated by this node is located in storage */
	uint8 *ptr;
};

struct EdgeMap {
	int16 edge1;
	int16 edge2;
};

struct KeyTraceMatch {
	uint8 *ptr;
	uint8 rank;
	int32 word_len;
};

struct EdgeKeyMap {
	uint8 n;
	uint8 *edge_codes;
};

struct IndexIgnoreList {
	uint8 n;
	uint8 *ignore_list;
};

struct Filter {
	/* --------------------------------------------------------
	 * Instance variables that maintains the state of this recognizer.
	 * -------------------------------------------------------- */
	/* Keyboard type */
	FilterType kbd_type;
	/* Recognition mode */
	FilterMode recognition_mode;
	/* Pointer to the object that attached the listener */
	void *init_callback_listener;
	/* The callback function to call when the recognizer is
	 * done initializing */
	filterInitCallback init_callback;
	/* The edge key map */
	EdgeKeyMap *edge_key_map;
	/* The index ignore list */
	IndexIgnoreList *index_ignore_list;
	/* The encoding table */
	EncodingTable *encoding_table;
	/* The edge map */
	EdgeMap *edge_map;
	/* The total number of patterns in storage */
	int32 pattern_count;
	/* pointer to the first encoded pattern in storage */
	uint8 *storage;
	/* Size of storage in bytes */
	int32 storage_size;
	/* pointer to the first index */
	Index *indices;
	/* The total number of indices */
	int32 index_count;
	/* The total number of extra patterns in storage */
	int32 extra_pattern_count;
	/* Pointer to the first encoded extra pattern in storage */
	uint8 *extra_storage;
	/* Size of extra storage in bytes */
	int32 extra_storage_size;
	/* Pointer to the first extra index */
	Index *extra_indices;
	/* The total number of extra indices */
	int32 extra_index_count;
	/* Shape sigma */
	float32 shape_sigma;
	/* Location sigma */
	float32 location_sigma;
	/* --------------------------------------------------------
	 * Scratch variables: used for temporary recurring computations to
	 * avoid use of static variable and/or excessive malloc-free calls
	 * -------------------------------------------------------- */
	Parameters *parameters;
	Index *search_indices;
	Neighbor *neighbors;
	uint8 *start_neighbors;
	uint8 *end_neighbors;
	Heap *heap;
	Result *parallel_results;
	float32 *probabilities;
	Result *partition1;
	Result *partition2;
	Result *reranked1;
	Result *reranked2;
	int32 parallel_results_count;
	FilterResults *result_set;
};

typedef struct WordPoint{
	int16	x;
	int16	y;
} WordPoint;


static int32 get_encoding_byte_count(const char *str, int32 n, const EncodingTable *encoding_table);
static int32 get_encoding_nodes_child_count(const EncodingTable *encoding_table);
static void create_flat_encoding_table_index(const EncodingTable *encoding_table, int32 *index);
static void fill_encoding_table_index(int32 *index, int indices_count, FILE *stream);
static int32 get_encoding_node_child_index_count_at(int32 ch, const int32 *index, int32 indices_count);
static void calibrate_encoding_table(const EncodingTable *encoding_table, FILE *stream);
static EncodingTable * create_encoding_table(const FilterTable* key_mapping_set, const char *ignore_chars, int32 n_ignore_chars);
static EdgeMap * create_edge_map(float32 row_break1, float32 row_break2);
static int32 get_str_encoding_index(const EncodingTable *encoding_table, int32 ec);
static int32 get_encoding_index(const EncodingTable *encoding_table, int16 x, int16 y);
static int32 get_unique_xy_pairs_count(const FilterTable *key_mapping_set);
static BOOL can_encode(const char *str, int32 n, EncodingTable *encoding_table);
static InternalFilterResult encode_string(const char *str, int32 n, uint8 *pat, uint8 ds, const EncodingTable *encoding_table, int32 *out_byte_count);
static int32 decode_points(const uint8 *in, int16 *out, const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list);
static int32 decode_indices(const uint8 *in, uint8 *out, const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list);
static uint8 get_last_encoded_char(const uint8 *in);

static char * memory_stream_data_buffer_getline(void *in_data_buffer, char *out_buffer);
static char * file_stream_data_buffer_getline(void *in_data_buffer, char *out_buffer);
static InternalFilterResult get_data_buffer_encoding_byte_count(void *data_buffer, data_buffer_getline getline, EncodingTable *encoding_table, int32 *out_byte_count, int32 *out_word_count);
static InternalFilterResult create_patterns_from_data_buffer(void *data_buffer, data_buffer_getline getline, uint8 *out_stg, int32 out_stg_size, EncodingTable *encoding_table, int32 word_count, int32 *out_pattern_count, BOOL activate_all);
static fpos_t skip_past_utf8_byte_order_mark(FILE *stream);
static void find_encoded_neighbors(EncodingTable *encoding_table, Neighbor *neighbors, int16 x, int16 y, uint8 *out, float32 radius, uint8 *n_keys);
static void get_indices(Filter *aFilter, int16 *unknown, Index *index, int32 *index_n, float32 n_start_point_searched_radius, float32 n_end_point_searched_radius);

static void fuzzy_key_search(Parameters *parameters, uint8 *stg, int32 pattern_count, EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 *unknown, Result *out_results, int32 *out_results_count);
static int key_trace_match_compar(const void *o1, const void *o2);
static uint8 find_encoded_key(EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 x, int16 y);
static void get_string_trace(EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 *pat, int32 n, uint8 *out_trace, int32 *out_n);
static BOOL word_inside_trace(uint8 *word, int32 n_word, uint8 *trace, int32 n_trace);
static void remove_string_trace_duplicates(uint8 *in_trace, int32 in_n, uint8 *out_trace, int32 *out_n);

static BOOL edge_match(uint8 *edge_crossings1, uint8 n1, uint8 *edge_crossings2, uint8 n2);
static EdgeKeyMap * create_edge_key_map(EdgeMap *edge_map, EncodingTable *encoding_table);
static void get_edge_crossings(uint8 *edge_codes, uint8 *trace, int32 n, uint8 *out_edge_crossings, uint8 *out_n_edge_crossings);

static BOOL is_in_ignore_chars(int32 c, char *ignore_chars, int32 n_ignore_chars);
static IndexIgnoreList * create_index_ignore_list(EncodingTable *encoding_table, char *ignore_chars, int32 n_ignore_chars);

static int32 get_ignore_characters_not_in_mapping_set_count(const FilterTable *key_mapping_set, const char *ignore_chars, int32 n_ignore_chars);
static void get_ignore_characters_not_in_mapping_set(const FilterTable *key_mapping_set, const char *ignore_chars, int32 n_ignore_chars, int32 *add_list);

static InternalFilterResult alloc_scratch(Filter* aFilter);
static InternalFilterResult create_indices(uint8 *raw_stg, uint8 *blank_storage, int32 pattern_count, int32 char_mapping_count, Index **aIndices, int32 *index_count);
static InternalFilterResult compute_index_values(Index *indices, int32 index_count, EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, EdgeKeyMap *edge_key_map, Parameters *parameters);
static void get_directional_tendency(int16 *pattern, int32 n, uint8 *out_x, uint8 *out_y, Parameters *parameters, uint8 mode);
static uint8 pack(uint8 *bitpairs, uint8 n);
static uint8 create_data_segment(uint8 rank, BOOL active, BOOL reserved);
static BOOL get_data_segment_active(uint8 ds);
static void set_data_segment_active(uint8 *ds, BOOL active);
static BOOL get_data_segment_remove(uint8 ds);
static void set_data_segment_remove(uint8 *ds, BOOL remove);
static BOOL get_data_segment_reserved(uint8 ds);
static void set_data_segment_reserved(uint8 *ds, BOOL reserved);
static uint8 get_data_segment_rank(uint8 ds);
static int neighbor_compar(const void *o1, const void *o2);
static int32 input_signal_to_pattern(const InputSignal *input_signal, int16 *pat, int32 max_len);
static float32 gauss_prob(float32 d, float32 sigma);
static float32 bi_alpha(float32 i, float32 n, float32 w);
static float32 shape_dist(int16 *pat1, int16 *pat2, register int32 n);
static int16 pattern_length(int16 *pat, int32 n);
static float32 distance_noapprox(float32 x1, float32 y1, float32 x2, float32 y2);
static float32 distance_array(int16 *p, int16 *q, int32 n);
static int16 distance(int16 x1, int16 y1, int16 x2, int16 y2);
static void rerank_results(Filter *aFilter, RankMethod rank_method);
static void rerank_results_partitioned(Filter *aFilter);
static void rerank_results_shifted(Filter *aFilter);
static void partition(Result *results, int32 result_count, Result *p1, Result *p2, int32 *n1, int32 *n2);
static void rerank_partition(Result *partition, int32 partition_count, Result *reranked);
static void rerank_segment(Result *segment, int32 n);
static int segment_compar(const void *o1, const void *o2);
static void parallel_channel_integration(Filter *aFilter, int16 *unknown);
static void prune_to_heap(Filter *aFilter, int16 *unknown, Index *search_indices, int32 n_search_indices);
static int result_compar(const void *o1, const void *o2);
static void shape_location_dist(int16 *unknown, int16 *candidate, int16 ucx, int16 ucy, int16 ccx, int16 ccy, float32 s, register int32 n, float32 *sd, float32 *ld);
//static void shape_location_dist(int16 *unknown, int16 *candidate, int16 cx, int16 cy, int16 dx, int16 dy, float32 s, register int32 n, float32 *sd, float32 *ld);
static void integrate(float32 *probabilities, int32 n);
static int16 center_x(Bounds *bounds);
static int16 center_y(Bounds *bounds);
static void get_bounds(int16 *pat, int32 n, Bounds *bounds);
static void resample(int16 *in, int16 *out, int32 n, int32 pts);
static float32 pts_per_seg(int16 *pat, int32 n, float32 segl, int16 *segment_buf);

static InternalFilterResult get_decoding_storage_byte_count(uint8 *storage, int32 pattern_count, EncodingTable *encoding_table, int32 *out_byte_count);
static InternalFilterResult decode_storage(uint8 *storage, int32 pattern_count, EncodingTable *encoding_table, char *out_buffer, int32 *out_string_count);

static void delete_encoding_table(EncodingTable *encoding_table);

static Heap * heap_create(int32 capacity);
static void heap_delete(Heap *heap);
static void heap_clear(Heap *heap);
static void heap_insert(Heap *heap, float32 distance, uint8 *ptr);
static void heap_sift_up(Heap *heap, int32 i);
static void heap_remove(Heap *heap);
static void heap_sift_down(Heap *heap, int32 i);
static void heap_dump(Heap *heap, Filter *aFilter);

static float32 pattern_length_noapprox(int16 *pat, int32 n);
static void resample_noapprox(int16 *in, int16 *out, int32 n, int32 pts);
static uint8 * get_system_pattern_ptr(Filter *aFilter, const char *str);
static uint8 * get_extra_pattern_ptr(Filter *aFilter, const char *str);
static void get_corner_from_signal(Filter *aFilter, int16 * signal, int32 n, int16 * corner, int32 * m);
static void find_corner(int16 * signal, int32 n, uint8 * corner);
static int16 edit_distance(int16 * in, int32 n, int16 * std, int32 m, int16 key_width, int16 key_height);
//static int16 compare_distance(int16 x1, int16 y1, int16 x2, int16 y2, int16 key_width, int16 key_height);
static WordPoint start_direction(int16 * signal, int32 n, int16 radius);


#ifdef FILTER_DEBUG
static void dump_encoding_table(const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list);
static void dump_storage(const uint8 *stg, int32 pattern_count, const EncodingTable *encoding_table);
static void dump_trace(EncodingTable *encoding_table, uint8 *trace, int32 n);
#endif

#ifdef FILTER_LOGGER
static void log_play(FILE *stream);
static SwiInputSignal * log_read_input_signal(FILE *stream);
static FILE * log_open();
static void log_close(FILE *stream);
static void log_input_signal(FILE *stream, const SwiInputSignal *input_signal);
static void log_results(FILE *stream, const Result *results, int32 n, const EncodingTable *encoding_table);
static void log_probabilities(FILE *stream, float32 *probabilities, int32 n, const EncodingTable *encoding_table, HeapNode *nodes);
#endif

Filter* createFilter()
{
	Filter* filter = q_malloc(sizeof(Filter));
	if (filter == NULL)	return NULL;
    
	filter->init_callback = NULL;
	memset(filter, 0, sizeof(Filter));
	filter->recognition_mode = MODE1;
	filter->parameters = q_malloc(sizeof(Parameters));
	if (filter->parameters == NULL) {
        q_free(filter);
		return NULL;
	}
    
    //Init the parameters
	filter->parameters->n_start_point_searched_keys = 9;
	filter->parameters->n_end_point_searched_keys = 9;
	filter->parameters->n_sample_points = 80;
	filter->parameters->n_pruning_sample_points = 30;
	filter->parameters->key_width = 0;
	filter->parameters->key_height = 0;
	return filter;
}

void setParamsForFilter(Filter* aFilter, uint8 aP1, uint8 aP2, uint8 aP3, uint8 aP4, int16 aP5, int16 aP6)
{
    if (aFilter == NULL) return;
	aFilter->parameters->n_start_point_searched_keys = aP1;
	aFilter->parameters->n_end_point_searched_keys = aP2;
	aFilter->parameters->n_sample_points = aP3;
	aFilter->parameters->n_pruning_sample_points = aP4;
	aFilter->parameters->key_width = aP5;
	aFilter->parameters->key_height = aP6;
}

void destroyFilter(Filter* aFilter)
{
	int32 i, n;
    if (aFilter == NULL) return;
    
	aFilter->init_callback = NULL;
	aFilter->init_callback_listener = NULL;
	q_free(aFilter->parameters);
	aFilter->parameters = NULL;
	q_free(aFilter->storage);
	aFilter->storage = NULL;

	q_free(aFilter->extra_storage);
	aFilter->extra_storage = NULL;

	n = aFilter->index_count;
	for (i = 0; i < n; i++) {
		q_free(aFilter->indices[i].lengths);
		q_free(aFilter->indices[i].pecs);
		q_free(aFilter->indices[i].xdts);
		q_free(aFilter->indices[i].ydts);
		q_free(aFilter->indices[i].x0dts);
		q_free(aFilter->indices[i].y0dts);
		aFilter->indices[i].len = 0;
		aFilter->indices[i].ptr = NULL;
	}
	n = aFilter->extra_index_count;
	if (n > 0) {
		for (i = 0; i < n; i++) {
			q_free(aFilter->extra_indices[i].lengths);
			q_free(aFilter->extra_indices[i].pecs);
			q_free(aFilter->extra_indices[i].xdts);
			q_free(aFilter->extra_indices[i].ydts);
			q_free(aFilter->extra_indices[i].x0dts);
			q_free(aFilter->extra_indices[i].y0dts);
			aFilter->extra_indices[i].len = 0;
			aFilter->extra_indices[i].ptr = NULL;
		}
	}
	q_free(aFilter->indices);
	aFilter->indices = NULL;
	q_free(aFilter->extra_indices);
	aFilter->extra_indices = NULL;
	q_free(aFilter->parallel_results);
	aFilter->parallel_results = NULL;
	n = MAX_RESULTS;
	for (i = 0; i < n; i++) {
		q_free(aFilter->result_set->iResults[i].iResult);
	}
	aFilter->result_set->iNumOfResults = 0;
	q_free(aFilter->result_set->iResults);
	aFilter->result_set->iResults = NULL;
	q_free(aFilter->result_set);
	aFilter->result_set = NULL;
	delete_encoding_table(aFilter->encoding_table);
	q_free(aFilter->encoding_table);
	aFilter->encoding_table = NULL;
	q_free(aFilter->edge_map);
	aFilter->edge_map = NULL;
	aFilter->edge_key_map->n = 0;
	q_free(aFilter->edge_key_map->edge_codes);
	aFilter->edge_key_map->edge_codes = NULL;
	q_free(aFilter->edge_key_map);
	aFilter->edge_key_map = NULL;
	aFilter->index_ignore_list->n = 0;
	q_free(aFilter->index_ignore_list->ignore_list);
	aFilter->index_ignore_list->ignore_list = NULL;
	q_free(aFilter->index_ignore_list);
	aFilter->index_ignore_list = NULL;
	/* Scratch variables */
	q_free(aFilter->search_indices);
	aFilter->search_indices = NULL;
	q_free(aFilter->neighbors);
	aFilter->neighbors = NULL;
	q_free(aFilter->start_neighbors);
	aFilter->start_neighbors = NULL;
	q_free(aFilter->end_neighbors);
	aFilter->end_neighbors = NULL;
	q_free(aFilter->partition1);
	aFilter->partition1 = NULL;
	q_free(aFilter->partition2);
	aFilter->partition2 = NULL;
	q_free(aFilter->reranked1);
	aFilter->reranked1 = NULL;
	q_free(aFilter->reranked2);
	aFilter->reranked2 = NULL;
	q_free(aFilter->probabilities);
	aFilter->probabilities = NULL;
	heap_delete(aFilter->heap);
	aFilter->heap = NULL;
	aFilter->pattern_count = 0;
	aFilter->index_count = 0;
	aFilter->parallel_results_count = 0;
	aFilter->shape_sigma = 0.0f;
	aFilter->location_sigma = 0.0f;
	q_free(aFilter);
	aFilter = NULL;
}

FilterLoadResult loadFilterFromBuffer(void* aBuffer)
{
	int32 i, j, n, m;
	Filter *filter;
	char head[8];
	char tail[8];
	int32 t;
	FilterLoadResult load_result;
//	ptrdiff_t delta_offset;
	int32 delta_offset;
	int32 enode_count;
	EncodingNode *enode;

	ram_stream* ram_stream = q_malloc(sizeof(ram_stream));
	ram_stream->iBuffer = aBuffer;
	ram_stream->iCurrentPtr = aBuffer;
	ram_stream->iReadCount = 0;


	load_result.iFilter = NULL;
	filter = createFilter();
    
	/* Read the head */
	for (i = 0; i < 8; i++) {
		t = ram_read_int8(ram_stream);
		head[i] = (char)t;
	}

	if (memcmp("filter", head, 6) != 0) {
		/* Bad head */
		load_result.iLoadResult = LOAD_ERROR_BAD_HEAD;
		return load_result;
	}

	if (memcmp("10", head + 6, 2) != 0) {
		/* Bad version */
		load_result.iLoadResult = LOAD_ERROR_BAD_VERSION;
		return load_result;
	}

	/* Read the data */
	/* Read the parameters */

	filter->parameters->n_start_point_searched_keys = ram_read_uint8(ram_stream);
	filter->parameters->n_end_point_searched_keys = ram_read_uint8(ram_stream);
	filter->parameters->n_pruning_sample_points = ram_read_uint8(ram_stream);
	filter->parameters->n_sample_points = ram_read_uint8(ram_stream);
	filter->parameters->key_width = ram_read_int16(ram_stream);
	filter->parameters->key_height = ram_read_int16(ram_stream);
	filter->kbd_type = ram_read_int32(ram_stream);
	filter->recognition_mode = ram_read_int32(ram_stream);
	
	/* Read the index ignore list */
	filter->index_ignore_list = q_malloc(sizeof(IndexIgnoreList));
	filter->index_ignore_list->n = ram_read_uint8(ram_stream);

	n = filter->index_ignore_list->n;
	filter->index_ignore_list->ignore_list = q_malloc(sizeof(uint8) * n);
	for (i = 0; i < n; i++) {
		filter->index_ignore_list->ignore_list[i] = ram_read_uint8(ram_stream);
	}
	/* Read the edge key map */
	filter->edge_key_map = q_malloc(sizeof(EdgeKeyMap));
	filter->edge_key_map->n = ram_read_uint8(ram_stream);

	n = filter->edge_key_map->n;
	filter->edge_key_map->edge_codes = q_malloc(sizeof(uint8) * n);
	for (i = 0; i < n; i++) {
		filter->edge_key_map->edge_codes[i] = ram_read_uint8(ram_stream);
	}

	/* Read the edge map */
	filter->edge_map = q_malloc(sizeof(EdgeMap));
	filter->edge_map->edge1 = ram_read_int16(ram_stream);
	filter->edge_map->edge2 = ram_read_int16(ram_stream);

	/* Read the encoding table */
	filter->encoding_table = q_malloc(sizeof(EncodingTable));
	/* Read and build the encoding table */
	filter->encoding_table->n = ram_read_int32(ram_stream);

	filter->encoding_table->encoding_nodes = q_malloc(filter->encoding_table->n * sizeof(EncodingNode));
	filter->encoding_table->x_components = q_malloc(filter->encoding_table->n * sizeof(int16));
	filter->encoding_table->y_components = q_malloc(filter->encoding_table->n * sizeof(int16));
	/* Initialize all component values to zero */
	memset(filter->encoding_table->encoding_nodes, 0, filter->encoding_table->n * sizeof(EncodingNode));
	memset(filter->encoding_table->x_components, 0, filter->encoding_table->n * sizeof(int16));
	memset(filter->encoding_table->y_components, 0, filter->encoding_table->n * sizeof(int16));
	n = filter->encoding_table->n;
	/* Read x components */
	for (i = 0; i < n; i++) {
		filter->encoding_table->x_components[i] = ram_read_int16(ram_stream);
	}

	/* Read y components */
	for (i = 0; i < n; i++) {
		filter->encoding_table->y_components[i] = ram_read_int16(ram_stream);
	}

	/* Read the encoding nodes */
	for (i = 0; i < n; i++) {
		/* Read an integer that tells us how many encoded characters this encoding node has */
		enode_count = ram_read_int32(ram_stream);

		/* Read the first encoding node */
		//enode = q_malloc( sizeof(EncodingNode));
		//fread(&enode->ec, sizeof(int32), 1, stream);
		//fread(&enode.ec, sizeof(int32), 1, stream);
		filter->encoding_table->encoding_nodes[i].ec = ram_read_int32(ram_stream);
		filter->encoding_table->encoding_nodes[i].next = NULL;
		/* Now read all the remaining encoded characters */
		for (j = 1; j < enode_count; j++) {
			/* Find the last node */
			enode = &(filter->encoding_table->encoding_nodes[i]);
			while (enode->next != NULL) {
				enode = enode->next;
			}
			enode->next = q_malloc(sizeof(EncodingNode));
			enode->next->ec = ram_read_int32(ram_stream);
		}
	}
	/* Read the patterns in storage */
	filter->pattern_count = ram_read_int32(ram_stream);
	n = ram_read_int32(ram_stream);

	filter->storage = q_malloc(n * sizeof(uint8));
	for (i = 0; i < n; i++) {
		filter->storage[i] = ram_read_uint8(ram_stream);
	}
	/* Read the index tables */
	filter->index_count = ram_read_int32(ram_stream);
	n = filter->index_count;
	filter->indices = q_malloc(n * sizeof(Index));
	for (i = 0; i < n; i++) {
		delta_offset = ram_read_int32(ram_stream);
		filter->indices[i].ptr = filter->storage + delta_offset;
		filter->indices[i].len = ram_read_int32(ram_stream);

		m = filter->indices[i].len;
		if (m > 0) {
			filter->indices[i].pecs = q_malloc(m * sizeof(uint8));
			filter->indices[i].xdts = q_malloc( m * sizeof(uint8));
			filter->indices[i].ydts = q_malloc(m * sizeof(uint8));
			filter->indices[i].x0dts = q_malloc(m * sizeof(uint8));
			filter->indices[i].y0dts = q_malloc(m * sizeof(uint8));
			filter->indices[i].lengths = q_malloc(m * sizeof(int16));
			/* Read the patterns' encoded edge crossings for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].pecs[j] = ram_read_uint8(ram_stream);
			}
			/* Read the patterns' xdts for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].xdts[j] = ram_read_uint8(ram_stream);
			}
			/* Read the patterns' ydts for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].ydts[j] = ram_read_uint8(ram_stream);
			}
			/* Read the patterns' x0dts for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].x0dts[j] = ram_read_uint8(ram_stream);
			}
			/* Read the patterns' y0dts for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].y0dts[j] = ram_read_uint8(ram_stream);
			}
			/* Read the patterns' lengths for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].lengths[j] = ram_read_int16(ram_stream);
			}
		}
	}
	/* Read parameters */
	filter->shape_sigma = ram_read_float32(ram_stream);
	filter->location_sigma = ram_read_float32(ram_stream);

	/* Read the tail */
	for (i = 0; i < 8; i++) {
		t = ram_read_uint8(ram_stream);
		tail[i] = (char)t;
	}

	if (memcmp("filter", tail, 6) != 0) {
		/* Bad tail */
		load_result.iLoadResult = LOAD_ERROR_BAD_TAIL;
		return load_result;
	}
	if (memcmp("ed", tail + 6, 2) != 0) {
		/* Bad version */
		load_result.iLoadResult = LOAD_ERROR_BAD_VERSION;
		return load_result;
	}
	alloc_scratch(filter);
	load_result.iLoadResult = LOAD_SUCCESS;
	load_result.iFilter = filter;

	q_free(ram_stream);
	return load_result;
}

FilterLoadResult loadFilterFromFile(FILE* aFile)
{
	int32 i, j, n, m;
	Filter *filter;
	char head[8];
	char tail[8];
	int32 t;
	FilterLoadResult load_result;
//	ptrdiff_t delta_offset;
	int32 delta_offset;
	int32 enode_count;
	EncodingNode *enode;

	load_result.iFilter = NULL;
	filter = createFilter();
	/* Read the head */
	for (i = 0; i < 8; i++) {
		t = fgetc(aFile);
		if (t == EOF) {
			/* Bad head */
			load_result.iLoadResult = LOAD_ERROR_BAD_HEAD;
			return load_result;
		}
		head[i] = (char)t;
	}
	if (memcmp("filter", head, 6) != 0) {
		/* Bad head */
		load_result.iLoadResult = LOAD_ERROR_BAD_HEAD;
		return load_result;
	}
	if (memcmp("10", head + 6, 2) != 0) {
		/* Bad version */
		load_result.iLoadResult = LOAD_ERROR_BAD_VERSION;
		return load_result;
	}
	/* Read the data */
	/* Read the parameters */
	fread(&(filter->parameters->n_start_point_searched_keys), sizeof(uint8), 1, aFile);
	fread(&(filter->parameters->n_end_point_searched_keys), sizeof(uint8), 1, aFile);
	fread(&(filter->parameters->n_pruning_sample_points), sizeof(uint8), 1, aFile);
	fread(&(filter->parameters->n_sample_points), sizeof(uint8), 1, aFile);
	filter->parameters->key_width = io_read_int16(aFile);
	filter->parameters->key_height = io_read_int16(aFile);

	/* Read the keyboard type */
	filter->kbd_type = io_read_int32(aFile);

	/* Read the recognition mode */
	filter->recognition_mode = io_read_int32(aFile);

	/* Read the index ignore list */
	filter->index_ignore_list = q_malloc( sizeof(IndexIgnoreList));
	fread(&(filter->index_ignore_list->n), sizeof(uint8), 1, aFile);
	n = filter->index_ignore_list->n;
	filter->index_ignore_list->ignore_list = q_malloc( sizeof(uint8) * n);
	for (i = 0; i < n; i++) {
		fread(&(filter->index_ignore_list->ignore_list[i]), sizeof(uint8), 1, aFile);
	}
	/* Read the edge key map */
	filter->edge_key_map = q_malloc( sizeof(EdgeKeyMap));
	fread(&(filter->edge_key_map->n), sizeof(uint8), 1, aFile);
	n = filter->edge_key_map->n;
	filter->edge_key_map->edge_codes = q_malloc( sizeof(uint8) * n);
	for (i = 0; i < n; i++) {
		fread(&(filter->edge_key_map->edge_codes[i]), sizeof(uint8), 1, aFile);
	}
	/* Read the edge map */
	filter->edge_map = q_malloc( sizeof(EdgeMap));
	filter->edge_map->edge1 = io_read_int16(aFile);
	filter->edge_map->edge2 = io_read_int16(aFile);

	/* Read the encoding table */
	filter->encoding_table = q_malloc( sizeof(EncodingTable));
	/* Read and build the encoding table */
	filter->encoding_table->n = io_read_int32(aFile);

	filter->encoding_table->encoding_nodes = q_malloc( filter->encoding_table->n * sizeof(EncodingNode));
	filter->encoding_table->x_components = q_malloc( filter->encoding_table->n * sizeof(int16));
	filter->encoding_table->y_components = q_malloc( filter->encoding_table->n * sizeof(int16));
	/* Initialize all component values to zero */
	memset(filter->encoding_table->encoding_nodes, 0, filter->encoding_table->n * sizeof(EncodingNode));
	memset(filter->encoding_table->x_components, 0, filter->encoding_table->n * sizeof(int16));
	memset(filter->encoding_table->y_components, 0, filter->encoding_table->n * sizeof(int16));
	n = filter->encoding_table->n;
	/* Read x components */
	for (i = 0; i < n; i++) {
		filter->encoding_table->x_components[i] = io_read_int16(aFile);
	}

	/* Read y components */
	for (i = 0; i < n; i++) {
		filter->encoding_table->y_components[i] = io_read_int16(aFile);
	}
	/* Read the encoding nodes */
	for (i = 0; i < n; i++) {
		/* Read an integer that tells us how many encoded characters this encoding node has */
		enode_count = io_read_int32(aFile);

		/* Read the first encoding node */
		//enode = q_malloc( sizeof(EncodingNode));
		//fread(&enode->ec, sizeof(int32), 1, stream);
		//fread(&enode.ec, sizeof(int32), 1, stream);

		filter->encoding_table->encoding_nodes[i].ec = io_read_int32(aFile);
		filter->encoding_table->encoding_nodes[i].next = NULL;
		/* Now read all the remaining encoded characters */
		for (j = 1; j < enode_count; j++) {
			/* Find the last node */
			enode = &(filter->encoding_table->encoding_nodes[i]);
			while (enode->next != NULL) {
				enode = enode->next;
			}
			enode->next = q_malloc( sizeof(EncodingNode));
			enode->next->ec = io_read_int32(aFile);
		}
	}
	/* Read the patterns in storage */
	filter->pattern_count = io_read_int32(aFile);

	n = io_read_int32(aFile);
	filter->storage = q_malloc( n * sizeof(uint8));
	for (i = 0; i < n; i++) {
		filter->storage[i] = fgetc(aFile);
	}
	/* Read the index tables */
	filter->index_count = io_read_int32(aFile);

	n = filter->index_count;
	filter->indices = q_malloc( n * sizeof(Index));
	for (i = 0; i < n; i++) {
		delta_offset = io_read_int32(aFile);
		filter->indices[i].ptr = filter->storage + delta_offset;
		filter->indices[i].len = io_read_int32(aFile);

		m = filter->indices[i].len;
		if (m > 0) {
			filter->indices[i].pecs = q_malloc( m * sizeof(uint8));
			filter->indices[i].xdts = q_malloc( m * sizeof(uint8));
			filter->indices[i].ydts = q_malloc( m * sizeof(uint8));
			filter->indices[i].x0dts = q_malloc( m * sizeof(uint8));
			filter->indices[i].y0dts = q_malloc( m * sizeof(uint8));
			filter->indices[i].lengths = q_malloc( m * sizeof(int16));
			/* Read the patterns' encoded edge crossings for the index position */
			for (j = 0; j < m; j++) {
				fread(&(filter->indices[i].pecs[j]), sizeof(uint8), 1, aFile);
			}
			/* Read the patterns' xdts for the index position */
			for (j = 0; j < m; j++) {
				fread(&(filter->indices[i].xdts[j]), sizeof(uint8), 1, aFile);
			}
			/* Read the patterns' ydts for the index position */
			for (j = 0; j < m; j++) {
				fread(&(filter->indices[i].ydts[j]), sizeof(uint8), 1, aFile);
			}
			/* Read the patterns' x0dts for the index position */
			for (j = 0; j < m; j++) {
				fread(&(filter->indices[i].x0dts[j]), sizeof(uint8), 1, aFile);
			}
			/* Read the patterns' y0dts for the index position */
			for (j = 0; j < m; j++) {
				fread(&(filter->indices[i].y0dts[j]), sizeof(uint8), 1, aFile);
			}
			/* Read the patterns' lengths for the index position */
			for (j = 0; j < m; j++) {
				filter->indices[i].lengths[j] = io_read_int16(aFile);
			}
		}
	}
	/* Read parameters */
	filter->shape_sigma = io_read_float32(aFile);
	filter->location_sigma = io_read_float32(aFile);

	/* Read the tail */
	for (i = 0; i < 8; i++) {
		t = fgetc(aFile);
		if (t == EOF) {
			/* Bad tail */
			load_result.iLoadResult = LOAD_ERROR_BAD_TAIL;
			return load_result;
		}
		tail[i] = (char)t;
	}
	if (memcmp("filter", tail, 6) != 0) {
		/* Bad tail */
		load_result.iLoadResult = LOAD_ERROR_BAD_TAIL;
		return load_result;
	}
	if (memcmp("ed", tail + 6, 2) != 0) {
		/* Bad version */
		load_result.iLoadResult = LOAD_ERROR_BAD_VERSION;
		return load_result;
	}
	alloc_scratch(filter);
	load_result.iLoadResult = LOAD_SUCCESS;
	load_result.iFilter = filter;
	return load_result;
}

int32 storeFilterIntoFile(Filter* aFilter, FILE* aFile)
{
	int32 i, j, n, m;
	int32 bytes_written;
//	ptrdiff_t delta_offset;
	int32 delta_offset;
	EncodingNode enode;
	int32 enode_count;

	bytes_written = 0;
	fputc('f', aFile);
	fputc('i', aFile);
	fputc('l', aFile);
	fputc('t', aFile);
	fputc('e', aFile);
	fputc('r', aFile);
	fputc('1', aFile);
	fputc('0', aFile);
	bytes_written+= 8;
	/* Write the parameters */
	bytes_written+= fwrite(&(aFilter->parameters->n_start_point_searched_keys), sizeof(uint8), 1, aFile) * sizeof(uint8);
	bytes_written+= fwrite(&(aFilter->parameters->n_end_point_searched_keys), sizeof(uint8), 1, aFile) * sizeof(uint8);
	bytes_written+= fwrite(&(aFilter->parameters->n_pruning_sample_points), sizeof(uint8), 1, aFile) * sizeof(uint8);
	bytes_written+= fwrite(&(aFilter->parameters->n_sample_points), sizeof(uint8), 1, aFile) * sizeof(uint8);
	bytes_written+= io_store_int16(aFile, aFilter->parameters->key_width);
	bytes_written+= io_store_int16(aFile, aFilter->parameters->key_height);
	
	/* Write the keyboard type */
	bytes_written+= io_store_int32(aFile, aFilter->kbd_type);
	/* Write the recognition mode */
	bytes_written+= io_store_int32(aFile, aFilter->recognition_mode);
	/* Write the index ignore list */
	bytes_written+= fwrite(&(aFilter->index_ignore_list->n), sizeof(uint8), 1, aFile) * sizeof(uint8);
	n = aFilter->index_ignore_list->n;
	for (i = 0; i < n; i++) {
		bytes_written+= fwrite(&(aFilter->index_ignore_list->ignore_list[i]), sizeof(uint8), 1, aFile) * sizeof(uint8);
	}
	/* Write  the edge key map */
	bytes_written+= fwrite(&(aFilter->edge_key_map->n), sizeof(uint8), 1, aFile) * sizeof(uint8);
	n = aFilter->edge_key_map->n;
	for (i = 0; i < n; i++) {
		bytes_written+= fwrite(&(aFilter->edge_key_map->edge_codes[i]), sizeof(uint8), 1, aFile) * sizeof(uint8);
	}
	/* Write down the edge map */
	bytes_written+= io_store_int16(aFile, aFilter->edge_map->edge1);
	bytes_written+= io_store_int16(aFile, aFilter->edge_map->edge2);

	/* Procedure to write down encoding table to disk begins */
	/* First write down the number of encoding nodes */
	n = aFilter->encoding_table->n;
	bytes_written+= io_store_int32(aFile, n);

	/* Write down x components */
	for (i = 0; i < n; i++) {
		bytes_written+= io_store_int16(aFile, aFilter->encoding_table->x_components[i]);
	}
	/* Write down y components */
	for (i = 0; i < n; i++) {
		bytes_written+= io_store_int16(aFile, aFilter->encoding_table->y_components[i]);
	}
	/* Write down the encoding nodes */
	for (i = 0; i < n; i++) {
		enode = aFilter->encoding_table->encoding_nodes[i];
		/* Count the number of encoded characters at the encoding node */
		enode_count = 1;
		while (enode.next != NULL) {
			enode_count++;
			enode = *(enode.next);
		}
		/* Write down the number of encoded characters at the encoding node */
		bytes_written+= io_store_int32(aFile, enode_count);

		/* Write down all the encoded characters at the encoding node */		
		enode = aFilter->encoding_table->encoding_nodes[i];
		bytes_written+= io_store_int32(aFile, enode.ec);

		while (enode.next != NULL) {
			bytes_written+= io_store_int32(aFile, enode.next->ec);
			enode = *(enode.next);
		}

	}
	n = aFilter->pattern_count;
	bytes_written+= io_store_int32(aFile, n);

	n = aFilter->storage_size;
	bytes_written+= io_store_int32(aFile, n);

	for (i = 0; i < n; i++) {
		fputc(aFilter->storage[i], aFile);
		bytes_written++;
	}
	n = aFilter->index_count;
	bytes_written+= io_store_int32(aFile, n);

	for (i = 0; i < n; i++) {
		delta_offset = aFilter->indices[i].ptr - aFilter->storage;
		bytes_written+= io_store_int32(aFile, delta_offset);

		bytes_written+= io_store_int32(aFile, aFilter->indices[i].len);

		m = aFilter->indices[i].len;
		/* Write the patterns' packed edge crossings at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= fwrite(&(aFilter->indices[i].pecs[j]), sizeof(uint8), 1, aFile) * sizeof(uint8);
		}
		/* Write the patterns' xdts at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= fwrite(&(aFilter->indices[i].xdts[j]), sizeof(uint8), 1, aFile) * sizeof(uint8);
		}
		/* Write the patterns' ydts at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= fwrite(&(aFilter->indices[i].ydts[j]), sizeof(uint8), 1, aFile) * sizeof(uint8);
		}
		/* Write the patterns' x0dts at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= fwrite(&(aFilter->indices[i].x0dts[j]), sizeof(uint8), 1, aFile) * sizeof(uint8);
		}
		/* Write the patterns' y0dts at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= fwrite(&(aFilter->indices[i].y0dts[j]), sizeof(uint8), 1, aFile) * sizeof(uint8);
		}
		/* Write the patterns' lengths at the index position */
		for (j = 0; j < m; j++) {
			bytes_written+= io_store_int16(aFile, aFilter->indices[i].y0dts[j]);
		}
	}

	bytes_written+= io_store_float32(aFile, aFilter->shape_sigma);
	bytes_written+= io_store_float32(aFile, aFilter->location_sigma);

	fputc('f', aFile);
	fputc('i', aFile);
	fputc('l', aFile);
	fputc('t', aFile);
	fputc('e', aFile);
	fputc('r', aFile);
	fputc('e', aFile);
	fputc('d', aFile);
	bytes_written+= 8;
	return bytes_written;
}

/*
 *
 * Allocates space for all the scratch buffers required for internal
 * calculations inside the Filter object.
 *
 * Only call this function once for a specific Filter object, otherwise
 * a memory leak may occur. This function assumes all internal buffers
 * of the Filter object are previously unallocated.
 *
 * *aFilter - a WordPointer to a created recognizer object
 *
 * Returns status of completion.
 */
static InternalFilterResult alloc_scratch(Filter* aFilter)
{
	int32 i;

	aFilter->search_indices = q_malloc( aFilter->encoding_table->n * aFilter->encoding_table->n * sizeof(Index) * 2);
	if (aFilter->search_indices == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->neighbors = q_malloc( aFilter->encoding_table->n * sizeof(Neighbor));
	if (aFilter->neighbors == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->start_neighbors = q_malloc( aFilter->encoding_table->n * sizeof(uint8));
	if (aFilter->start_neighbors == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->end_neighbors = q_malloc( aFilter->encoding_table->n * sizeof(uint8));
	if (aFilter->end_neighbors == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->heap = heap_create(HEAP_CAPACITY);
	if (aFilter->heap == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->probabilities = q_malloc( HEAP_CAPACITY * 2 * sizeof(float32));
	if (aFilter->probabilities == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->parallel_results = q_malloc( (HEAP_CAPACITY + 1) * sizeof(Result));
	if (aFilter->parallel_results == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->partition1 = q_malloc( MAX_RESULTS * sizeof(Result));
	if (aFilter->partition1 == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->partition2 = q_malloc( MAX_RESULTS * sizeof(Result));
	if (aFilter->partition2 == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->reranked1 = q_malloc( MAX_RESULTS * sizeof(Result));
	if (aFilter->reranked1 == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->reranked2 = q_malloc( MAX_RESULTS * sizeof(Result));
	if (aFilter->reranked2 == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->result_set = q_malloc( sizeof(FilterResults));
	if (aFilter->result_set == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	aFilter->result_set->iNumOfResults = 0;
	aFilter->result_set->iResults = q_malloc( MAX_RESULTS * sizeof(FilterResult));
	if (aFilter->result_set->iResults == NULL) {
		return MEMORY_ALLOC_FAILURE;
	}
	for (i = 0; i < MAX_RESULTS; i++) {
		aFilter->result_set->iResults[i].iResult = q_malloc( MAX_BYTES_PER_RESULT);
		if (aFilter->result_set->iResults[i].iResult == NULL) {
			return MEMORY_ALLOC_FAILURE;
		}
	}
	aFilter->extra_pattern_count = 0;
	aFilter->extra_pattern_count = 0;
	aFilter->extra_index_count = 0;
	aFilter->extra_storage = NULL;
	aFilter->extra_indices = NULL;
	return FILTER_SUCCESS;
}

void setFilterMode(Filter* aFilter, FilterMode aFilterMode)
{
    if (aFilter == NULL) return;
	aFilter->recognition_mode = aFilterMode;
}

static EdgeMap * create_edge_map(float32 row_break1, float32 row_break2)
{
	EdgeMap *edge_map;

	edge_map = q_malloc( sizeof(EdgeMap));
	if (edge_map == NULL) {
		return NULL;
	}
	edge_map->edge1 = (int16)row_break1;
	edge_map->edge2 = (int16)row_break2;
	return edge_map;
}

void initFilter(Filter* aFilter, FILE* aFile, const FilterTable* aFilterTable, FilterParams* aFilterParms)
{
	InternalFilterResult result;
	int32 byte_count, word_count, pattern_count;
	uint8 *tmp_pattern_stg;
	fpos_t stream_pos;

	aFilter->kbd_type = aFilterParms->iFilterType;
	aFilter->encoding_table = create_encoding_table(aFilterTable, aFilterTable->iIgnorChars, aFilterTable->iNumOfIgnorChars);
	aFilter->index_ignore_list = create_index_ignore_list(aFilter->encoding_table, aFilterTable->iIgnorChars, aFilterTable->iNumOfIgnorChars);
	calibrate_encoding_table(aFilter->encoding_table, aFile);
	aFilter->edge_map = create_edge_map(aFilterParms->iDivider1, aFilterParms->iDivider2);
	aFilter->edge_key_map = create_edge_key_map(aFilter->edge_map, aFilter->encoding_table);
	aFilter->parameters->key_width = (int16)aFilterParms->iWidth;
	aFilter->parameters->key_height = (int16)aFilterParms->iHeight;
	aFilter->shape_sigma = SHAPE_SIGMA * aFilter->parameters->key_width;
	aFilter->location_sigma = LOCATION_SIGMA * aFilter->parameters->key_width;
	
	/* Ensure stream is past UTF-8 BOM, and get the stream position at that WordPoint */
	stream_pos = skip_past_utf8_byte_order_mark(aFile);
	result = get_data_buffer_encoding_byte_count(aFile, file_stream_data_buffer_getline, aFilter->encoding_table, &byte_count, &word_count);
	/* Rewind stream to the same stream position as before the previous read */
	fsetpos(aFile, &stream_pos);
	tmp_pattern_stg = q_malloc( byte_count);
	result = create_patterns_from_data_buffer(aFile, file_stream_data_buffer_getline, tmp_pattern_stg, byte_count, aFilter->encoding_table, word_count, &pattern_count, FALSE);
	aFilter->storage = q_malloc( byte_count);
	aFilter->storage_size = byte_count;
	aFilter->pattern_count = pattern_count;
	memset(aFilter->storage, 0, byte_count);
	result = create_indices(tmp_pattern_stg, aFilter->storage, aFilter->pattern_count, aFilter->encoding_table->n, &(aFilter->indices), &(aFilter->index_count));
	result = compute_index_values(aFilter->indices, aFilter->index_count, aFilter->encoding_table, aFilter->index_ignore_list, aFilter->edge_key_map, aFilter->parameters);
	q_free(tmp_pattern_stg);
	alloc_scratch(aFilter);
	/* Done initializing */
	if (aFilter->init_callback != NULL) {
		aFilter->init_callback(aFilter->init_callback_listener, aFilter, 1.0);
	}
}

/*
 *
 * Returns TRUE if the given string can be enoded using our encoding table.
 *
 * *str - a WordPointer to string, the string does not need to be NULL-terminated.
 * n - the number of characters in the input string, which will be part of the pattern's
 * header
 * *encoding_table - a WordPointer to the encoding table
 *
 * The return value is TRUE if the string can be encoded, FALSE otherwise.
 */
static BOOL can_encode(const char *str, int32 n, EncodingTable *encoding_table)
{
	int32 i;
	int32 index;
	int32 c;
	
	if (n < 2) {
		return FALSE;
	}
	for (i = 0; i < n; i++) {
		c = utf8_char_at(str, i);
		index = get_str_encoding_index(encoding_table, c);
		if (index == -1) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * Returns the number of bytes required to encode the given string.
 *
 * The string does not need to be NULL-terminated.
 *
 * *str - WordPointer to the string
 * n - the number of characters in the string
 * *encoding_table - WordPointer to the encoding table
 *
 * Returns the number of bytes required to encode the given string.
 */
static int32 get_encoding_byte_count(const char *str, int32 n, const EncodingTable *encoding_table)
{
	int32 i, c;
	int32 byte_count;
	int32 index;

	byte_count = 0;
	for (i = 0; i < n; i++) {
		c = utf8_char_at(str, i);
		index = get_str_encoding_index(encoding_table, c);
		/* No index for character, character is unencodable, signal failure */
		if (index == -1) {
			return -1;
		}
		/* If the head node contains the character, it requires one byte to encode */
		if (c == encoding_table->encoding_nodes[index].ec) {
			byte_count++;
		}
		/* Otherwise, the character must be encoded using two bytes */
		else {
			byte_count+= 2;
		}
	}
	return byte_count;
}

/*
 * Returns whether the specified byte represents an escaped character.
 *
 * ch - the byte to investigate
 *
 * Returns whether the specified byte represents an escaped character.
 */
static BOOL get_encoded_char_escaped(uint8 ch)
{
	return (ch & ECHAR_ESCAPED_MASK) == ECHAR_ESCAPED_MASK;
}

/*
 * Returns the index (in [0,127]) for an encoded byte in the encoding table.
 *
 * ch - the encoded byte
 * 
 * Returns the index in [0,127].
 */
static uint8 get_encoded_char_index(uint8 ch)
{
	ch<<= 1;
	ch>>= 1;
	return ch;
}

/*
 * Returns an encoded byte based on an index in [0,127] and an escape
 * flag.
 *
 * index - the index in [0,127]
 * escaped - an on/off flag
 * 
 * Returns an encoded byte.
 */
static uint8 encode_char(uint8 index, BOOL escaped)
{
	if (escaped) {
		index|= ECHAR_ESCAPED_MASK;
	}
	else {
		index&= ~ECHAR_ESCAPED_MASK;
	}
	return index;
}

/*
 * Encodes a string into a pattern in memory.
 *
 * *str - a WordPointer to an UTF-8 string
 * n - the number of characters in the string
 * *pat - a WordPointer to the output buffer (where the encoded byte pattern
 * will be written)
 * ds - the data segment that will be part of the pattern
 * *encoding_table - the encoding table to use when encoding the pattern
 * *out_byte_count - a WordPointer to an integer. This integer will hold
 * the number of bytes written to the output buffer WordPointed to by *pat
 *
 * Returns status of completion.
 */
static InternalFilterResult encode_string(const char *str, int32 n, uint8 *pat, uint8 ds, const EncodingTable *encoding_table, int32 *out_byte_count)
{
	int32 i, c;
	int32 byte_count;
	int32 index, count, out_index;
	EncodingNode *enode;

	byte_count = 0;
	out_index = PATTERN_HEADER_SIZE;
	for (i = 0; i < n; i++) {
		c = utf8_char_at(str, i);
		index = get_str_encoding_index(encoding_table, c);
		/* No index for character, character is unencodable, signal failure */
		if (index == -1) {
			return ERROR_UNENCODABLE_STRING;
		}
		/* If the head node contains the character, it requires one byte to encode */
		if (c == encoding_table->encoding_nodes[index].ec) {
			byte_count++;
			pat[out_index] = encode_char((uint8)index, FALSE);
			out_index++;
		}
		/* Otherwise, the character must be encoded using two bytes */
		else {
			byte_count+= 2;
			pat[out_index] = encode_char((uint8)index, TRUE);
			/* Find child node position for character at index and store it */
			enode = &(encoding_table->encoding_nodes[index]);
			count = 0;
			while (enode->next != NULL && enode->ec != c) {
				enode = enode->next;
				count++;
			}
			pat[out_index + 1] = (uint8)count;
			out_index+= 2;
		}
	}
	/* Set the byte count in range [0,255] */
	pat[0] = (uint8)byte_count;
	/* Set the 8-bit data segment */
	pat[1] = ds;
	*out_byte_count = byte_count;
	return FILTER_SUCCESS;
}

/*
 * Returns the first byte of the last encoded character in the given encoded pattern.
 * 
 * Note that this is not necessarily the very last byte in the encoded pattern,
 * since some bytes contain escape codes and need a subsequent byte to be
 * properly decoded.
 * 
 * *in - a WordPointer to a pattern whose last encoded char should be found
 *
 * Returns the first byte of the last encoded character.
 */
static uint8 get_last_encoded_char(const uint8 *in)
{
	int32 i, byte_count;
	uint8 ch = '\0';

	byte_count = *in;
	i = 0;
	while(i < byte_count) {
		ch = in[PATTERN_HEADER_SIZE + i];
		if (get_encoded_char_escaped(ch)) {
			i+= 2;
		}
		else {
			i++;
		}
	}
	return ch;
}

/*
 * Decodes the supplied encoded pattern into a UTF-8 string.
 *
 * IMPORTANT: The output buffer must be zeroed out before calling this function.
 *
 * *in - a WordPointer to the input pattern to decode
 * *out - a WordPointer to an output buffer to write out the decoded UTF-8 string to
 * *encoding_table - a WordPointer to the encoding table to use when decoding the pattern
 *
 */
static void decode_string(const uint8 *in, char *out, const EncodingTable *encoding_table)
{
	int32 i, j, byte_count, str_index;
	uint8 ch, child_depth;
	EncodingNode *enode;

	byte_count = *in;
	str_index = 0;
	i = 0;
	while(i < byte_count) {
		ch = in[PATTERN_HEADER_SIZE + i];
		enode = &(encoding_table->encoding_nodes[get_encoded_char_index(ch)]);
		if (get_encoded_char_escaped(ch)) {
			child_depth = in[PATTERN_HEADER_SIZE + i + 1];
			/* Fast-forward to the target child depth of the encoding node */
			for (j = 0; j < child_depth; j++) {
				enode = enode->next;
			}
			i+= 2;
		}
		else {
			i++;
		}
		utf8_set_char_at(out, str_index, enode->ec);
		str_index++;
	}
	utf8_set_char_at(out, str_index, '\0');
}

static BOOL is_in_ignore_chars(int32 c, char *ignore_chars, int32 n_ignore_chars)
{
	register int32 i;

	for (i = 0; i < n_ignore_chars; i++) {
		if (c == utf8_char_at(ignore_chars, i)) {
			return TRUE;
		}
	}
	return FALSE;
}

static IndexIgnoreList * create_index_ignore_list(EncodingTable *encoding_table, char *ignore_chars, int32 n_ignore_chars)
{
	uint8 i, n;
	EncodingNode *encoding_nodes;
	EncodingNode *enode;
	IndexIgnoreList *index_ignore_list;
	BOOL brk_early;

	index_ignore_list = q_malloc( sizeof(IndexIgnoreList));
	if (index_ignore_list == NULL) {
		return NULL;
	}
	n = encoding_table->n;
	index_ignore_list->n = n;
	index_ignore_list->ignore_list = q_malloc( sizeof(uint8) * n);
	encoding_nodes = encoding_table->encoding_nodes;
	for (i = 0; i < n; i++) {
		brk_early = FALSE;
		enode = &(encoding_nodes[i]);
		if (is_in_ignore_chars(enode->ec, ignore_chars, n_ignore_chars)) {
			index_ignore_list->ignore_list[i] = 1;
			continue;
		}
		while (enode->next != NULL) {
			enode = enode->next;
			if (is_in_ignore_chars(enode->ec, ignore_chars, n_ignore_chars)) {
				index_ignore_list->ignore_list[i] = 1;
				brk_early = TRUE;
				break;
			}
		}
		if (brk_early) {
			continue;
		}
		index_ignore_list->ignore_list[i] = 0;
	}
	return index_ignore_list;
}

/*
 * Decodes the supplied encoded pattern into a geometric pattern
 * (series of [x,y] coordinates).
 *
 * *in - a WordPointer to the input pattern to decode
 * *out - a WordPointer to an output buffer to write out the geometric pattern to
 * *encoding_table - a WordPointer to the encoding table to use when decoding the pattern
 *
 * Returns the number of WordPoints decoded into a geometric pattern.
 */
static int32 decode_WordPoints(const uint8 *in, int16 *out, const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list)
{
	int32 i, byte_count, pts_index;
	uint8 index;
	int16 *x_components, *y_components;

	x_components = encoding_table->x_components;
	y_components = encoding_table->y_components;
	byte_count = *in;
	pts_index = 0;
	i = 0;
	while(i < byte_count) {
		index = in[PATTERN_HEADER_SIZE + i];
		if (get_encoded_char_escaped(index)) {
			/* Skip one byte */
			i+= 2;
		}
		else {
			i++;
		}
		index = get_encoded_char_index(index);
		if (index_ignore_list->ignore_list[index] == 1) {
			continue;
		}
		out[pts_index * 2] = x_components[index];
		out[(pts_index * 2) + 1] = y_components[index];
		pts_index++;
	}
	return pts_index;
}

static int32 decode_indices(const uint8 *in, uint8 *out, const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list)
{
	int32 i, byte_count, j;
	uint8 index;

	byte_count = *in;
	j = 0;
	i = 0;
	while(i < byte_count) {
		index = in[PATTERN_HEADER_SIZE + i];
		if (get_encoded_char_escaped(index)) {
			/* Skip one byte */
			i+= 2;
		}
		else {
			i++;
		}
		index = get_encoded_char_index(index);
		if (index_ignore_list->ignore_list[index] == 1) {
			continue;
		}
		out[j++] = (uint8)index;
	}
	return j;
}

static int32 get_ignore_characters_not_in_mapping_set_count(const FilterTable *key_mapping_set, const char *ignore_chars, int32 n_ignore_chars)
{
	int32 i, j, n, count;
	int32 c, ic;
	BOOL found;

	n = key_mapping_set->iNumOfFilterItems;
	count = 0;
	for (i = 0; i < n_ignore_chars; i++) {
		ic = utf8_char_at(ignore_chars, i);
		found = FALSE;
		for (j = 0; j < n; j++) {
			c = utf8_char_at(key_mapping_set->iFilterItems[j].iLabel, 0);
			if (c == ic) {
				found = TRUE;
				break;
			}
		}
		if (!found) {
			count++;
		}
	}
	return count;
}

static void get_ignore_characters_not_in_mapping_set(const FilterTable *key_mapping_set, const char *ignore_chars, int32 n_ignore_chars, int32 *add_list)
{
	int32 i, j, n, index;
	int32 c, ic;
	BOOL found;

	n = key_mapping_set->iNumOfFilterItems;
	index = 0;
	for (i = 0; i < n_ignore_chars; i++) {
		ic = utf8_char_at(ignore_chars, i);
		found = FALSE;
		for (j = 0; j < n; j++) {
			c = utf8_char_at(key_mapping_set->iFilterItems[j].iLabel, 0);
			if (c == ic) {
				found = TRUE;
				break;
			}
		}
		if (!found) {
			add_list[index++] = ic;
		}
	}
}

/*
 * Creates an encoding table from a set of key mappings.
 *
 * *key_mapping_set - the set of key mappings
 *
 * Returns a WordPointer to a new encoding table for the supplied set of key mappings.
 */
static EncodingTable * create_encoding_table(const FilterTable *key_mapping_set, const char *ignore_chars, int32 n_ignore_chars)
{
	int32 i, c, n, unique_xy_pairs, encoding_index;
	int16 x, y;
	char *label;
	int32 added_toplevel_entries;
	EncodingTable *encoding_table;
	EncodingNode *enode, *curnode;
	int32 icount;
	int32 add_chars[127];

	n = key_mapping_set->iNumOfFilterItems;
	/* Create the encoding_table on the heap */
	encoding_table = q_malloc( sizeof(EncodingTable));
	if (encoding_table == NULL) {
		/* Memory allocation failure */
		return NULL;
	}
	/* Find all unique (x,y) pairs in the key mappings */
	unique_xy_pairs = get_unique_xy_pairs_count(key_mapping_set);
	/* Check if there are index-ignore chars that needs to be added to the table */
	icount = get_ignore_characters_not_in_mapping_set_count(key_mapping_set, ignore_chars, n_ignore_chars);
	encoding_table->n = unique_xy_pairs + icount;
	/* Allocate space for the encoding nodes that represent possible strings
	 * for unique (x,y) pairs
	 */
	encoding_table->encoding_nodes = q_malloc( encoding_table->n * sizeof(EncodingNode));
	encoding_table->x_components = q_malloc( encoding_table->n * sizeof(int16));
	encoding_table->y_components = q_malloc( encoding_table->n * sizeof(int16));
	/* Initialize all component values to zero */
	memset(encoding_table->encoding_nodes, 0, encoding_table->n * sizeof(EncodingNode));
	memset(encoding_table->x_components, 0, encoding_table->n * sizeof(int16));
	memset(encoding_table->y_components, 0, encoding_table->n * sizeof(int16));
	added_toplevel_entries = 0;
	for (i = 0; i < n; i++) {
		label = key_mapping_set->iFilterItems[i].iLabel;
		c = utf8_char_at(label, 0);
		if (c == 0) {
			/* Not a permissable character to map */
			continue;
		}
		x = (int16)key_mapping_set->iFilterItems[i].iPosX;
		y = (int16)key_mapping_set->iFilterItems[i].iPosY;
		/* Search for a matching encoding node */
		encoding_index = get_encoding_index(encoding_table, x, y);
		if (encoding_index == -1) {
			/* No match (therefore a new entry) */
			encoding_table->x_components[added_toplevel_entries] = x;
			encoding_table->y_components[added_toplevel_entries] = y;
			encoding_table->encoding_nodes[added_toplevel_entries].ec = c;
			/* No children yet */
			encoding_table->encoding_nodes[added_toplevel_entries].next = NULL;
			added_toplevel_entries++;
		}
		else {
			/* We have an index */
			EncodingNode encoding_node = encoding_table->encoding_nodes[encoding_index];
			if (encoding_node.ec == '\0') {
				/* Create an encoding node */
				encoding_table->encoding_nodes[encoding_index].ec = c;
				/* No children yet */
				encoding_table->encoding_nodes[encoding_index].next = NULL;
			}
			else {
				/* Add a new child */
				enode = q_malloc( sizeof(EncodingNode));
				enode->ec = c;
				/* The new child has no children yet */
				enode->next = NULL;
				/* Walk the already existing encoding node (and its children) */
				curnode = &(encoding_table->encoding_nodes[encoding_index]);
				while (curnode->next != NULL) {
					curnode = curnode->next;
				}
				/* Add the new child at the end of the chain */
				curnode->next = enode;
			}
		}
	}
	/* Add any index ignore chars that weren't part of the mapping set to the table */
	n = encoding_table->n - icount;
	if (icount > 0) {
		get_ignore_characters_not_in_mapping_set(key_mapping_set, ignore_chars, n_ignore_chars, add_chars);
		for (i = 0; i < icount; i++) {
			encoding_table->x_components[n + i] = SHRT_MIN;
			encoding_table->y_components[n + i] = SHRT_MIN;
//			encoding_table->x_components[n + i] = 0;
//			encoding_table->y_components[n + i] = 0;
			encoding_table->encoding_nodes[n + i].ec = add_chars[i];
			encoding_table->encoding_nodes[n + i].next = NULL;
		}
	}
	return encoding_table;
}

static void delete_encoding_table(EncodingTable *encoding_table)
{
	register int32 i;
	int32 n;
	EncodingNode *root, *node, *prev;

	n = encoding_table->n;
	/* For each encoding node, walk the children and make sure they
	 * are all freed. Note that the root node should not be freed as
	 * it is part of an array which is freed after this procedure.
	 */
	for (i = 0; i < n; i++) {
		/* Get the root encoding node in the encoding table for this
		 * index position.
		 */
		root = &(encoding_table->encoding_nodes[i]);
		/* Now walk the chain of children as far as possible and free
		 * the last child. Repeat this procedure until the root node
		 * has no children.
		 */
		while (root->next != NULL) {
			/* Current node */
			node = root->next;
			/* Previous node */
			prev = root;
			/* While the current node has a child, jump to that child
			 * and make the previous node WordPoint to what was the current
			 * node.
			 */
			while (node->next != NULL) {
				prev = node;
				node = node->next;
			}
			/* After the while-loop we are the child farthest away in the
			 * chain, so we free it.
			 */
			q_free(node);
			/* This also means that the previous node which WordPointed to the
			 * child should WordPoint to null.
			 */
			prev->next = NULL;
			/* Now repeat the procedure by walking the root node as far as
			 * possible, until the root node has no children.
			 */
		}
	}
	/* Delete the bottom-level arrays */
	q_free(encoding_table->encoding_nodes);
	q_free(encoding_table->x_components);
	q_free(encoding_table->y_components);
}

/*
 * Returns the total number of Unicode characters that exist in the
 * supplied encoding table.
 *
 * *encoding_table - the encoding table to scan
 *
 * Returns the total number of Unicode characters that are encoded in the
 * supplied encoding table.
 */
static int32 get_encoding_nodes_child_count(const EncodingTable *encoding_table)
{
	int32 i, n, count;
	EncodingNode *enode;

	count = 0;
	n = encoding_table->n;
	for (i = 0; i < n; i++) {
		enode = &(encoding_table->encoding_nodes[i]);
		count++;
		while (enode->next != NULL) {
			enode = enode->next;
			count++;
		}
	}
	return count;
}

/*
 * Fills out the array WordPointed to by *index with all characters present
 * in the supplied encoding table on any even positions and writes the
 * value zero at all odd positions in the array.
 *
 * The supplied array *index must have space for 2 * the total number of
 * characters in the supplied encoding table.
 *
 * *encoding_table - the encoding table to scan
 * *index - a WordPointer to the output buffer
 *
 */
static void create_flat_encoding_table_index(const EncodingTable *encoding_table, int32 *index)
{
	int32 i, j, n, ec;
	EncodingNode *enode;

	n = encoding_table->n;
	j = 0;
	for (i = 0; i < n; i++) {
		enode = &(encoding_table->encoding_nodes[i]);
		ec = enode->ec;
		index[j * 2] = ec;
		index[(j * 2) + 1] = 0;
		j++;
		while (enode->next != NULL) {
			enode = enode->next;
			ec = enode->ec;
			index[j * 2] = ec;
			index[(j * 2) + 1] = 0;
			j++;
		}
	}
}

/*
 * Reads the supplied stream and fills the supplied index with the count
 * of all characters in the index.
 *
 * The index must be an array where all even entries represent a Unicode (UTF-8)
 * character and all subsequent odd entries represent the count of said character
 * in the stream. The counts should be initially zeroed out unless incremental
 * counting of characters in the supplied stream is desired.
 *
 * *index - a WordPointer to the index
 * indices_count - the number of entries (indices) WordPointed to by *index
 * *stream - the UTF-8 encoded stream to read
 *
 */
static void fill_encoding_table_index(int32 *index, int indices_count, FILE *stream)
{
	fpos_t stream_pos;
	int utf8h1, utf8h2, utf8h3;
	char linebuf[MAX_BYTES_PER_LEXICON_LINE];
	int32 ch, i, n, j, m;

	/* Get the marker for the initial stream position */
	fgetpos(stream, &stream_pos);
	/* Read the first three bytes and check if they are byte-order mark
	 * for UTF-8
	 */
	utf8h1 = fgetc(stream);
	utf8h2 = fgetc(stream);
	utf8h3 = fgetc(stream);
	if (utf8h1 == 0xef && utf8h2 == 0xbb && utf8h3 == 0xbf) {
		/* If they are, do nothing. Stream is alreay processed past
		   the byte order mark */
	}
	else {
		/* Reverse stream position, first three bytes were not part
		   of a header */
		fsetpos(stream, &stream_pos);
	}
	m = indices_count * 2;
	/* Go through the stream and count occurances of different characters in the index */
	while(fgets(linebuf, MAX_BYTES_PER_LEXICON_LINE, stream) != NULL) {
		n = utf8_strlen(linebuf) - 1; /* Don't care about newline */
		for (i = 0; i < n; i++) {
			/* Get the current character */
			ch = utf8_char_at(linebuf, i);
			/* Search the index position for ch in the index */
			for (j = 0; j < m; j++) {
				if (ch == index[j * 2]) {
					/* Found character, increase its count */
					index[(j * 2) + 1]++;
					break;
				}
			}
		}
	}
	/* Reset stream to original position */
	fsetpos(stream, &stream_pos);
}

/*
 * Returns the count for a specific character in the given index.
 *
 * ch - the character to search for
 * *index - a WordPointer to the index
 * indices_count - the number of entries (indices) WordPointed to by *index
 *
 * Returns the given character's count in the index.
 */
static int32 get_encoding_node_child_index_count_at(int32 ch, const int32 *index, int32 indices_count)
{
	int32 i, n;

	n = indices_count * 2;
	for (i = 0; i < n; i++) {
		if (ch == index[i * 2]) {
			return index[(i * 2) + 1];
		}
	}
	return -1;
}

/*
 * Re-arranges the order of the supplied encoding table's encoding nodes' children
 * so that the most common character is first in the change.
 *
 * *encoding_table - a WordPointer to the encoding table
 * *stream - a WordPointer to the stream to read
 *
 */
static void calibrate_encoding_table(const EncodingTable *encoding_table, FILE *stream)
{
	int32 c;
	int32 *index;
	int32 i, n, count, j, k;
	EncodingNode *head, *enode;
	int32 chbuf[MAX_ENCODING_NODE_CHILD_COUNT];
	int32 freq, max_freq;
	int32 ch;
	int32 max_index;

	c = get_encoding_nodes_child_count(encoding_table);
	index = q_malloc( sizeof(int32) * c * 2);
	create_flat_encoding_table_index(encoding_table, index);
	fill_encoding_table_index(index, c, stream);
	n = encoding_table->n;
	for (i = 0; i <n; i++) {
		head = &(encoding_table->encoding_nodes[i]);
		enode = head;
		/* Count the number of children at the head node */
		count = 0;
		while (enode->next != NULL) {
			enode = enode->next;
			count++;
		}
		/* If there are no children, proceed without structural modification */
		if (count == 0) {
			continue;
		}
		/* Else, sort the children by their counts in the index */
		count = 0;
		/* Step 1, copy all child nodes (including head) over to a buffer. */
		enode = head;
		chbuf[count++] = enode->ec;
		while (enode->next != NULL) {
			enode = enode->next;
			chbuf[count++] = enode->ec;
		}
		/* Step 2, find each */
		enode = head;
		for (j = 0; j < count; j++) {
			max_freq = INT_MIN;
			max_index = -1;
			for (k = 0; k < count; k++) {
				if (chbuf[k] != 0) {
					ch = chbuf[k];
					freq = get_encoding_node_child_index_count_at(ch, index, c);
					if (freq > max_freq) {
						max_freq = freq;
						max_index = k;
					}
				}
			}
			enode->ec = chbuf[max_index];
			chbuf[max_index] = 0;
			enode = enode->next;
		}
	}
	q_free(index);
}

/*
 * Returns the index in the encoding table used to encode the supplied character.
 *
 * *encoding_table - a WordPointer to the encoding table
 * c - the character to find
 *
 * Returns the index in the encoding table used to encode the supplied character,
 * or -1 if the character was not found in the supplied encoding table.
 */
static int32 get_str_encoding_index(const EncodingTable *encoding_table, int32 c)
{
	uint8 i, n;
	EncodingNode *encoding_nodes;
	EncodingNode *enode;

	n = encoding_table->n;
	encoding_nodes = encoding_table->encoding_nodes;
	for (i = 0; i < n; i++) {
		enode = &(encoding_nodes[i]);
		if (enode->ec == c) {
			return i;
		}
		while (enode->next != NULL) {
			enode = enode->next;
			if (enode->ec == c) {
				return i;
			}
		}
	}
	return -1;
}

/*
 * Returns the index in the encoding table used to encode the supplied coordinate.
 *
 * *encoding_table - a WordPointer to the encoding table
 * x - the horizontal component of the query coordinate
 * y - the vertical component of the query coordinate
 *
 * Returns the index in the encoding table used to encode the supplied coordinate,
 * or -1 if the character was not found in the supplied encoding table.
 */
static int32 get_encoding_index(const EncodingTable *encoding_table, int16 x, int16 y)
{
	uint8 i, n;
	int16 *x_components, *y_components;

	n = encoding_table->n;
	x_components = encoding_table->x_components;
	y_components = encoding_table->y_components;
	for (i = 0; i < n; i++) {
		if (x_components[i] == x && y_components[i] == y) {
			return i;
		}
	}
	return -1;
}

/*
 * Returns the number of unique (x,y) pairs in the key mapping set.
 * 
 * This is a support function for creating an encoder.
 *
 * *key_mapping_set - a mapping between strings and WordPoints
 */
static int32 get_unique_xy_pairs_count(const FilterTable *key_mapping_set)
{
	int32 i, j, n;
	int16 x, y;
	int16 x_index[127];
	int16 y_index[127];
	BOOL proceed_flag;

	for (i = 0; i < 127; i++) {
		x_index[i] = SHRT_MIN;
	}
	for (i = 0; i < 127; i++) {
		y_index[i] = SHRT_MIN;
	}
	n = key_mapping_set->iNumOfFilterItems;
	for (i = 0; i < n; i++) {
		x = (int16)key_mapping_set->iFilterItems[i].iPosX;
		y = (int16)key_mapping_set->iFilterItems[i].iPosY;
		proceed_flag = FALSE;
		/* See if this (x,y) position already exists */
		for (j = 0; j < 127; j++) {
			if (x_index[j] == x && y_index[j] == y) {
				/* Already registered this (x,y) */
				proceed_flag = TRUE;
				break;
			}
		}
		if (proceed_flag) {
			continue;
		}
		/* This (x,y) is not registered, add it last */
		for (j = 0; j < 127; j++) {
			if (x_index[j] == SHRT_MIN && y_index[j] == SHRT_MIN) {
				x_index[j] = x;
				y_index[j] = y;
				break;
			}
		}
	}
	/* Count the occurances in the indices */
	for (i = 0; i < 127; i++) {
		if (x_index[i] == SHRT_MIN && y_index[i] == SHRT_MIN) {
			return i;
		}
	}
	return 0;
}

BOOL remapFilter(Filter* aFilter, const FilterTable* aFilterTable, FilterParams* aFilterParms)
{
	register int32 i, j;
	int32 n, m;
	int32 c;
	EncodingTable *encoding_table;
	EncodingNode *node;
	int32 found_index;
	BOOL no_issues;
	InternalFilterResult result;

	no_issues = TRUE;
	aFilter->kbd_type = aFilterParms->iFilterType;
	encoding_table = aFilter->encoding_table;
	n = aFilterTable->iNumOfFilterItems;
	m = encoding_table->n;
	/* Go through all the new key mappings */
	for (i = 0; i < n; i++) {
		c = utf8_char_at(aFilterTable->iFilterItems[i].iLabel, 0);
		/* Find the index in the encoding table for the label */
		found_index = -1;
		for (j = 0; j < m; j++) {
			node = &(encoding_table->encoding_nodes[j]);
			if (c == node->ec) {
				found_index = j;
				break;
			}
			/* Check all children */
			while (node->next != NULL) {
				node = node->next;
				if (c == node->ec) {
					found_index = j;
					break;
				}
			}
			if (found_index > -1) {
				break;
			}
		}
		/* If not found the new key mapping set is invalid, fail immediately */
		if (found_index < 0) {
			no_issues = FALSE;
		}
		/* Change X, Y info at the index in the encoding table */
		encoding_table->x_components[found_index] = (int16)aFilterTable->iFilterItems[i].iPosX;
		encoding_table->y_components[found_index] = (int16)aFilterTable->iFilterItems[i].iPosY;
	}
	/* Update other coordinate-related parameters */
	if (aFilter->edge_map != NULL) {
		q_free(aFilter->edge_map);
		aFilter->edge_map = NULL;
	}
	if (aFilter->edge_key_map != NULL) {
		aFilter->edge_key_map->n = 0;
		q_free(aFilter->edge_key_map->edge_codes);
		aFilter->edge_key_map->edge_codes = NULL;
		q_free(aFilter->edge_key_map);
		aFilter->edge_key_map = NULL;
	}
	if (aFilter->index_ignore_list != NULL) {
		aFilter->index_ignore_list->n = 0;
		q_free(aFilter->index_ignore_list->ignore_list);
		aFilter->index_ignore_list->ignore_list = NULL;
		q_free(aFilter->index_ignore_list);
		aFilter->index_ignore_list = NULL;
	}
	aFilter->edge_map = create_edge_map(aFilterParms->iDivider1, aFilterParms->iDivider2);
	aFilter->edge_key_map = create_edge_key_map(aFilter->edge_map, aFilter->encoding_table);
	aFilter->index_ignore_list = create_index_ignore_list(aFilter->encoding_table, aFilterTable->iIgnorChars, aFilterTable->iNumOfIgnorChars);
	aFilter->parameters->key_width = (int16)aFilterParms->iWidth;
	aFilter->parameters->key_height = (int16)aFilterParms->iHeight;
	aFilter->shape_sigma = SHAPE_SIGMA * aFilter->parameters->key_width;
	aFilter->location_sigma = LOCATION_SIGMA * aFilter->parameters->key_width;
	/* Re-compute keyboard-topology dependent index values */
	result = compute_index_values(aFilter->indices, aFilter->index_count, aFilter->encoding_table, aFilter->index_ignore_list, aFilter->edge_key_map, aFilter->parameters);
	result = compute_index_values(aFilter->extra_indices, aFilter->extra_index_count, aFilter->encoding_table, aFilter->index_ignore_list, aFilter->edge_key_map, aFilter->parameters);
	return no_issues;
}

static char * memory_stream_data_buffer_getline(void *in_data_buffer, char *out_buffer)
{
	MemoryDataBuffer *mem_buf;
	int32 byte_count;

	mem_buf = (MemoryDataBuffer *)in_data_buffer;
	if (mem_buf->current_line_count < mem_buf->total_line_count) {
		byte_count = strlen(mem_buf->pointer) + 1;
		memcpy(out_buffer, mem_buf->pointer, byte_count);
		mem_buf->pointer+= byte_count;
		mem_buf->current_line_count++;
		return out_buffer;
	}
	return NULL;
}

static char * file_stream_data_buffer_getline(void *in_data_buffer, char *out_buffer)
{
	int32 n;
	int32 ch1, ch2;

	out_buffer = fgets(out_buffer, MAX_BYTES_PER_LEXICON_LINE, (FILE *)in_data_buffer);
	if (out_buffer == NULL) {
		return NULL;
	}
	/* Start remove-newline procedure */
	n = utf8_strlen(out_buffer);
	if (n > 1) {
		/* Check CR+LF */
		ch1 = utf8_char_at(out_buffer, n - 2);
		ch2 = utf8_char_at(out_buffer, n - 1);
		if (ch1 == 0x0d && ch2 == 0x0a) {
			utf8_set_char_at(out_buffer, n - 2, '\0');
			return out_buffer;
		}
	}
	if (n > 0) {
		ch1 = utf8_char_at(out_buffer, n - 1);
		/* Check LF, CR, NEL, FF, LS, PS */
		if (ch1 == 0x0a || ch1 == 0x0d || ch1 == 0x085 || ch1 == 0x0c || ch1 == 0x2028 || ch1 == 0x2029) {
			utf8_set_char_at(out_buffer, n - 1, '\0');
		}
	}
	return out_buffer;
}

/*
 * Modifies the stream to move the file position indicator beyond the UTF-8 byte mark
 * (if the UTF-8 byte mark is present), and returns an fpos_t object that
 * contains the file position indicator at that stream position.
 *
 * *stream - the UTF-8 encoded stream to read from
 *
 * Returns an fpos_t object that contains the file position indicator at the stream
 * position beyond a possible UTF-8 byte mark.
 */
static fpos_t skip_past_utf8_byte_order_mark(FILE *stream)
{
	fpos_t stream_pos;
	int utf8h1, utf8h2, utf8h3;

	/* Get the marker for the initial stream position */
	fgetpos(stream, &stream_pos);
	/* Read the first three bytes and check if they are byte-order mark
	 * for UTF-8
	 */
	utf8h1 = fgetc(stream);
	utf8h2 = fgetc(stream);
	utf8h3 = fgetc(stream);
	if (utf8h1 == 0xef && utf8h2 == 0xbb && utf8h3 == 0xbf) {
		/* First three bytes formed the UTF-8 byte order mark.
		 * Update the beginning-of-stream position variable so it WordPoints beyond
		 * the byte order mark.
		 */
		fgetpos(stream, &stream_pos);
	}
	else {
		/* First three bytes did not form the UTF-8 byte order mark.
		 * Reverse stream position to the original position.
		 */
		fsetpos(stream, &stream_pos);
	}
	return stream_pos;
}

/*
 * Returns the number of bytes required to encode the strings in the supplied data buffer.
 *
 * *data_buffer - the data buffer to read lines from
 * getline - a WordPointer to a function that can read a line from the *data_buffer
 *
 * Returns the number of bytes required
 */
static InternalFilterResult get_data_buffer_encoding_byte_count(void *data_buffer, data_buffer_getline getline, EncodingTable *encoding_table, int32 *out_byte_count, int32 *out_word_count)
{
	int32 pos, n, byte_count, word_count;
	char linebuf[MAX_BYTES_PER_LEXICON_LINE];

	byte_count = 0;
	word_count = 0;
	pos = 0;
	while(getline(data_buffer, linebuf) != NULL) {
		n = utf8_strlen(linebuf);
		if (can_encode(linebuf, n, encoding_table)) {
			byte_count+= get_encoding_byte_count(linebuf, n, encoding_table);
			word_count++;
		}
		else {
			//SWI_LOG(get_data_buffer_encoding_byte_count, ("Invalid token: '%s' at position: %d", linebuf, pos));
		}
		pos++;
	}
	*out_word_count = word_count;
	*out_byte_count = ((word_count * PATTERN_HEADER_SIZE) + byte_count) * sizeof(uint8);
	return FILTER_SUCCESS;
}

/*
 * Creates patterns in the supplied storage from the strings in the supplied data buffer.
 *
 * *data_buffer - the data buffer to read lines from
 * getline - a WordPointer to a function that can read a line from the *data_buffer
 *
 * Returns the number of bytes required
 */
static InternalFilterResult create_patterns_from_data_buffer(void *data_buffer, data_buffer_getline getline, uint8 *out_stg, int32 out_stg_size, EncodingTable *encoding_table, int32 word_count, int32 *out_pattern_count, BOOL activate_all)
{
	int32 n, rank, rank_active_passive_cutoff;
	int32 bcount, used_byte_count;
	InternalFilterResult result;
	char linebuf[MAX_BYTES_PER_LEXICON_LINE];
	uint8 transformed_rank;
	int32 pattern_count;

	rank = 1;
	used_byte_count = 0;
	pattern_count = 0;
//	rank_active_passive_cutoff = (int32)(word_count * ACTIVE_PARTITION_PERCENTAGE);
//	rank_active_passive_cutoff = 7000;
	rank_active_passive_cutoff = 15000;
	while(getline(data_buffer, linebuf) != NULL) {
		n = utf8_strlen(linebuf);
		if (n != 0 && can_encode(linebuf, n, encoding_table)) {
			/* Compute transformed smoothed rank in [0,63] */
			transformed_rank = 63 -(uint8)(floor(exp(-10 * ((float32)rank / (float32)word_count)) * 64));
			if (activate_all) {
				result = encode_string(linebuf, n, out_stg, create_data_segment(transformed_rank, TRUE, FALSE), encoding_table, &bcount);
			}
			else {
				result = encode_string(linebuf, n, out_stg, create_data_segment(transformed_rank, rank <= rank_active_passive_cutoff ? TRUE : FALSE, FALSE), encoding_table, &bcount);
			}
			if (result != FILTER_SUCCESS) {
				return result;
			}
			/* Jump bcount bytes for encoded pattern + PATTERN_HEADER_SIZE byte for length prefix */
			used_byte_count+= bcount + PATTERN_HEADER_SIZE;
			if (used_byte_count > out_stg_size) {
				return ERROR_BUFFER_TOO_SMALL;
			}
			out_stg+= bcount + PATTERN_HEADER_SIZE;
			pattern_count++;
			rank++;
		}
	}
	*out_pattern_count = pattern_count;
	return FILTER_SUCCESS;
}

/*
 * Computes and creates indices that indexes the patterns in a raw
 * unprocessed temporary storage area, and fills the recognizer's
 * preallocated storage area with patterns dictated by the index.
 *
 * IMPORTANT NOTES:
 *
 * 1. This function MUST ONLY be called immediately after create_raw_storage
 * 2. This function MUST ONLY be called ONCE during the entire lifespan of the
 * recognizer object.
 * 3. *raw_stg MUST contain patterns; the patterns may be in any order
 * 4. filter->storage MUST be preallocated
 * 5. The entire block WordPointed to by filter->storage MUST be initialized to
 * zero.
 * 6. The *raw_stg parameter will be invalid when this function returns.
 *
 * *filter - the recognizer object
 * *raw_stg - a WordPointer to an unprocessed storage area
 */
static InternalFilterResult create_indices(uint8 *raw_stg, uint8 *blank_storage, int32 pattern_count, int32 char_mapping_count, Index **aIndices, int32 *index_count)
{
	int32 i, n;
	uint8 *raw_stg_ptr, *stg, *pat_ptr;
	int32 sz;
	uint8 start_enc, end_enc;
	int32 cmc;
	Index *indices;
	Bucket *buckets;
	int32 offset;

	n = pattern_count;
	cmc = char_mapping_count;
	buckets = q_malloc( cmc * cmc * sizeof(Bucket));
	raw_stg_ptr = raw_stg;
	memset(buckets, 0, cmc * cmc * sizeof(Bucket));
	/* Step 1: Count occurances of all patterns with a given start and end encoding*/
	for (i = 0; i < n; i++) {
		sz = *raw_stg_ptr;
		if (sz == 0) {
			//SWI_LOG(create_indices, ("sz 0"));
		}
		/* Fetch the pattern's start and end encodings */
		start_enc = get_encoded_char_index(*(raw_stg_ptr + PATTERN_HEADER_SIZE));
		end_enc = get_encoded_char_index(get_last_encoded_char(raw_stg_ptr));
		offset = start_enc * cmc + end_enc;
		buckets[offset].n++;
		buckets[offset].size+= sz + PATTERN_HEADER_SIZE;
		/* Go to next pattern */
		raw_stg_ptr+= sz + PATTERN_HEADER_SIZE;
	}
	/* Step 2: Construct the index */
	indices = q_malloc( cmc * cmc * sizeof(Index));
	*aIndices = indices;
	/* Iterate over all the buckets and build the index */
	n = cmc * cmc;
	/* Important: Must iterate over *new* storage allocated, otherwise
	 * the WordPointers would WordPoint to the raw (invalid!) storage area at step 3!
	 */
	stg = blank_storage;
	for (i = 0; i < n; i++) {
		/* Get the bucket size of this index */
		sz = buckets[i].size;
		/* Store the WordPointer to this storage position */
		if (sz == 0) {
			indices[i].ptr = NULL;
		}
		else {
			indices[i].ptr = stg;
		}
		/* Store the number of patterns at this index */
		indices[i].len = buckets[i].n;
		/* Create space for index values */
		indices[i].pecs = q_malloc( sizeof(uint8) * indices[i].len);
		indices[i].xdts = q_malloc( sizeof(uint8) * indices[i].len);
		indices[i].ydts = q_malloc( sizeof(uint8) * indices[i].len);
		indices[i].x0dts = q_malloc( sizeof(uint8) * indices[i].len);
		indices[i].y0dts = q_malloc( sizeof(uint8) * indices[i].len);
		indices[i].lengths = q_malloc( sizeof(int16) * indices[i].len);
		/* Move the WordPointer to next position */
		stg+= sz;
	}
	/* Step 3: Asssign patterns to storage according to the index */
	stg = blank_storage;
	raw_stg_ptr = raw_stg;
	n = pattern_count;
	for (i = 0; i < n; i++) {
		sz = *raw_stg_ptr;
		/* Fetch the pattern's start and end encodings */
		//start_enc = *(raw_stg_ptr + PATTERN_HEADER_SIZE);
		//end_enc = *(raw_stg_ptr + PATTERN_HEADER_SIZE + sz - 1);
		start_enc = get_encoded_char_index(*(raw_stg_ptr + PATTERN_HEADER_SIZE));
		//end_enc = get_encoded_char_index(*(raw_stg_ptr + PATTERN_HEADER_SIZE + sz - 1));
		end_enc = get_encoded_char_index(get_last_encoded_char(raw_stg_ptr));
		offset = start_enc * cmc + end_enc;
		/* Get the index WordPointer */
		pat_ptr = indices[offset].ptr;
		/* Walk the WordPointer until a pattern with zero size is encountered. */
		while (*pat_ptr) {
			pat_ptr+= *pat_ptr + PATTERN_HEADER_SIZE;
		}
		memcpy(pat_ptr, raw_stg_ptr, sz + PATTERN_HEADER_SIZE);
		/* Go to next pattern */
		raw_stg_ptr+= sz + PATTERN_HEADER_SIZE;
	}
	*index_count = cmc * cmc;
	q_free(buckets);
	return FILTER_SUCCESS;
}

static InternalFilterResult compute_index_values(Index *indices, int32 index_count, EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, EdgeKeyMap *edge_key_map, Parameters *parameters)
{
	int32 i, j, n, m;
	Index index;
	uint8 *pattern_ptr;
	uint8 stg_sz;
	int16 pattern[MAX_CHARS_PER_STRING * 2];
	int16 pat_len;
	uint8 edge_crossings[MAX_CHARS_PER_STRING];
	uint8 n_edge_crossings;
	uint8 pec;
	uint8 xdt, ydt;
	uint8 x0dt, y0dt;
	int32 pat_index_len;
	uint8 index_pat[MAX_CHARS_PER_LEXICON_LINE];
	int16 spatial_len;

	n = index_count;
	for (i = 0; i < n; i++) {
		index = indices[i];
		m = index.len;
		pattern_ptr = index.ptr;
		for (j = 0; j < m; j++) {
			stg_sz = *pattern_ptr;
			/* Compute the edge crossings */
			pat_index_len = decode_indices(pattern_ptr, index_pat, encoding_table, index_ignore_list);
			memset(edge_crossings, 0, MAX_CHARS_PER_STRING);
			get_edge_crossings(edge_key_map->edge_codes, index_pat, pat_index_len, edge_crossings, &n_edge_crossings);
			/* Pack them into a single number and store it */
			//pec = pack(edge_crossings, n_edge_crossings);
			pec = pack(edge_crossings, 1);
			index.pecs[j] = pec;
			/* Compute directional constraint */
			pat_len = decode_WordPoints(pattern_ptr, pattern, encoding_table, index_ignore_list);
			get_directional_tendency(pattern, pat_len * 2, &xdt, &ydt, parameters, 1);
			index.xdts[j] = xdt;
			index.ydts[j] = ydt;
			get_directional_tendency(pattern, pat_len * 2, &x0dt, &y0dt, parameters, 2);
			index.x0dts[j] = x0dt;
			index.y0dts[j] = y0dt;
			/* Compute the spatial length */
			spatial_len = (int16)((float32)pattern_length(pattern, pat_len) / (float32)parameters->key_width);
			index.lengths[j] = spatial_len;
			pattern_ptr+= stg_sz + PATTERN_HEADER_SIZE;
		}
	}
	return FILTER_SUCCESS;
}

/*
 * Returns a preconfigured data segment based on given parameters.
 *
 * rank - a rank in the range [0,63]. No error checking is performed to validate
 * that the rank is within the valid range. A value outside the range will result
 * in an overflow or underflow.
 * active - if TRUE the active bit is set, if FALSE the active bit is cleared
 * reserved - if TRUE the reserved bit is set, if FALSE the reserved bit is cleared
 *
 * The return value is a data segment that is preconfigured according to the parameters
 * passed into this function.
 */
static uint8 create_data_segment(uint8 rank, BOOL active, BOOL reserved)
{
	uint8 ds;

	ds = rank;
	set_data_segment_active(&ds, active);
	set_data_segment_reserved(&ds, reserved);
	return ds;
}

/*
 * Returns whether a pattern's active bit is set or not.
 *
 * ds - a data segment
 *
 * The return value is TRUE if the active bit is set, FALSE otherwise.
 */
static BOOL get_data_segment_active(uint8 ds)
{
	return (ds & DS_ACTIVE_MASK) == DS_ACTIVE_MASK;
}

/*
 * Sets a pattern's active bit in the data segment WordPointed to.
 *
 * *ds - a WordPointer to a data segment
 * reserved - if TRUE the active bit is set, if FALSE the active bit is cleared
 */
static void set_data_segment_active(uint8 *ds, BOOL active)
{
	if (active) {
		(*ds)|= DS_ACTIVE_MASK;
	}
	else {
		(*ds)&= ~DS_ACTIVE_MASK;
	}
}

/*
 * Sets a pattern's remove bit in the data segment WordPointed to.
 *
 * *ds - a WordPointer to a data segment
 * remove - if TRUE the remove bit is set, if FALSE the remove bit is cleared
 */
static void set_data_segment_remove(uint8 *ds, BOOL remove)
{
	set_data_segment_reserved(ds, remove);
}

static BOOL get_data_segment_remove(uint8 ds)
{
	return get_data_segment_reserved(ds);
}

/*
 * Returns whether a pattern's reserved bit is set or not.
 *
 * ds - a data segment
 *
 * The return value is TRUE if the reserved bit is set, FALSE otherwise.
 */
static BOOL get_data_segment_reserved(uint8 ds)
{
	return (ds & DS_RESERVED_MASK) == DS_RESERVED_MASK;
}

/*
 * Sets a pattern's reserved bit in the data segment WordPointed to.
 *
 * *ds - a WordPointer to a data segment
 * reserved - if TRUE the reserved bit is set, if FALSE the reserve bit is cleared
 */
static void set_data_segment_reserved(uint8 *ds, BOOL reserved)
{
	if (reserved) {
		(*ds)|= DS_RESERVED_MASK;
	}
	else {
		(*ds)&= ~DS_RESERVED_MASK;
	}
}

/*
 * Returns a pattern's ranked based on the given data segment.
 *
 * ds - a data segment
 *
 * The return value is an integer rank value.
 */
static uint8 get_data_segment_rank(uint8 ds)
{
	ds<<= 2;
	ds>>= 2;
	return ds;
}

/* 
 * Converts an external input signal into an internal 2D-WordPoint pattern
 * sequence that can be used directly for recognition.
 *
 * *input_signal - the input signal to convert
 * *pat - the pattern to write (output buffer)
 * max_len - the maximum number of WordPoints that can be written to the output buffer
 *
 * The return value is the number of sample WordPoints in the input signal. Note that
 * this number may be higher than the number of converted sample WordPoints written to
 * the output buffer, if the output buffer is too small.
 */
static int32 input_signal_to_pattern(const InputSignal *input_signal, int16 *pat, int32 max_len)
{
	int32 i, n, m;
	SamplePoint pt;

	n = input_signal->iNumOfSamplePoints;
	m = n * 2;
	for(i = 0; i < m; i+= 2) {
		if (i < max_len) {
			pt = input_signal->iSamplePoints[i / 2];
			pat[i] = (int16)pt.iPosX;
			pat[i + 1] = (int16)pt.iPosY;
		}
	}
	return n;
}

static void find_encoded_neighbors(EncodingTable *encoding_table, Neighbor *neighbors, int16 x, int16 y, uint8 *out, float32 radius, uint8 * n_keys)
{
	int16 *x_components;
	int16 *y_components;
	int32 n, i;

	x_components = encoding_table->x_components;
	y_components = encoding_table->y_components;
	n = encoding_table->n;
	for (i = 0; i < n; i++) {
		neighbors[i].index = i;
		neighbors[i].distance = distance_noapprox(x, y, x_components[i], y_components[i]);
	}
	qsort(neighbors, n, sizeof(Neighbor), neighbor_compar);
	out[0] = neighbors[0].index;
	for (i = 1; i < n && neighbors[i].distance <= radius && i < MAX_N_NEIGHBORS; i++) {
		out[i] = neighbors[i].index;
	}
	*n_keys = i;
}

static int neighbor_compar(const void *o1, const void *o2)
{
	float32 v1, v2;
	v1 = ((Neighbor *)o1)->distance;
	v2 = ((Neighbor *)o2)->distance;
	if (v1 < v2) {
		return -1;
	}
	else if (v1 > v2) {
		return 1;
	}
	else {
		return 0;
	}
}

static void get_indices(Filter *aFilter, int16 *unknown, Index *index, int32 *index_n, float32 n_start_WordPoint_searched_radius, float32 n_end_WordPoint_searched_radius)
{
	int32 cmc, i, j, index_i;
	uint8 *start_nbors;
	uint8 *end_nbors;
	uint8 n, m;

	/* Search area (number of keys) around start WordPoint */
//	n = n_start_WordPoint_searched_keys;
	/* Search area (number of keys) around end WordPoint */
//	m = n_end_WordPoint_searched_keys;
	cmc = aFilter->encoding_table->n;
	start_nbors = aFilter->start_neighbors;
	end_nbors = aFilter->end_neighbors;
	memset(start_nbors, 0, aFilter->encoding_table->n);
	memset(end_nbors, 0, aFilter->encoding_table->n);
	find_encoded_neighbors(aFilter->encoding_table, aFilter->neighbors, unknown[0], unknown[1], start_nbors, n_start_WordPoint_searched_radius, &n);
	find_encoded_neighbors(aFilter->encoding_table, aFilter->neighbors, unknown[(aFilter->parameters->n_pruning_sample_points * 2) - 2],
		unknown[(aFilter->parameters->n_pruning_sample_points * 2) - 1], end_nbors, n_end_WordPoint_searched_radius, &m);
	/* Fill the index array with all combinations */
	index_i = 0;
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			index[index_i++] = aFilter->indices[start_nbors[i] * cmc + end_nbors[j]];
		}
	}
	/* Now the extra indices */
	if (aFilter->extra_index_count > 0) {
		for (i = 0; i < n; i++) {
			for (j = 0; j < m; j++) {
				index[index_i++] = aFilter->extra_indices[start_nbors[i] * cmc + end_nbors[j]];
			}
		}
	}
	if (aFilter->extra_index_count > 0) {
		*index_n = n * m * 2;
	}
	else {
		*index_n = n * m;
	}
}

FilterResults* filterWord(Filter* aFilter, const InputSignal* aInputSignal, const void* aReserved)
{
	FilterResults *rs;
	FilterResult *results;
	uint8 *pat_ptr;
	EncodingTable *encoding_table;
	int32 i, unknown_raw_len;
	Result *final_results;
	int32 final_result_count;
	int16 unknown_raw[10000]; // FIX ME, BUFFER OVERFLOW RISK
	int16 unknown_pruning[MAX_N_PRUNING_SAMPLE_POINTS * 2];
//	int16 unknown[MAX_N_SAMPLE_WordPointS * 2];
	Index *search_indices;
	int32 search_indices_n;
#ifdef FILTER_LOGGER
	FILE *log_stream;
#endif

#ifdef FILTER_LOGGER
	log_stream = fopen("C:\\test.log", "rb");
	log_play(log_stream);
	fclose(log_stream);
	log_stream = log_open();
	log_input_signal(log_stream, input_signal);
	log_close(log_stream);
#endif
	search_indices = aFilter->search_indices;
	memset(search_indices, 0, aFilter->encoding_table->n * aFilter->encoding_table->n * sizeof(Index));
	/* Preprocess input signal into a raw unknown pattern */
	unknown_raw_len = input_signal_to_pattern(aInputSignal, unknown_raw, 10000);
	/* Resample into a standard pattern */
	resample_noapprox(unknown_raw, unknown_pruning, unknown_raw_len, aFilter->parameters->n_pruning_sample_points);
	if (aFilter->recognition_mode == MODE1) {
		get_indices(aFilter, unknown_pruning, search_indices, &search_indices_n, 
			0.75f * aFilter->parameters->key_height, 1.0f * aFilter->parameters->key_height);
		prune_to_heap(aFilter, unknown_pruning, search_indices, search_indices_n);
		parallel_channel_integration(aFilter, unknown_pruning);
#ifdef FILTER_LOGGER
		log_stream = log_open();
		log_results(log_stream, aFilter->parallel_results, aFilter->parallel_results_count, aFilter->encoding_table);
		log_close(log_stream);
#endif
		if (aFilter->parallel_results_count == 0) {
			return NULL;
		//	fuzzy_key_search(filter->parameters, filter->storage, filter->pattern_count, filter->encoding_table, filter->index_ignore_list, unknown, filter->parallel_results, &(filter->parallel_results_count));
		}
		else {
			rerank_results(aFilter, RANK_ACTIVEPASSIVE_SHIFTED);
		}
	}
	else if (aFilter->recognition_mode == MODE2) {
	//	fuzzy_key_search(filter->parameters, filter->storage, filter->pattern_count, filter->encoding_table, filter->index_ignore_list, unknown, filter->parallel_results, &(filter->parallel_results_count));
		return NULL;
	}
	final_results = aFilter->parallel_results;
	final_result_count = MAX_RESULTS < aFilter->parallel_results_count ? MAX_RESULTS : aFilter->parallel_results_count;
	rs = aFilter->result_set;
	rs->iNumOfResults = final_result_count;
	results = rs->iResults;
	encoding_table = aFilter->encoding_table;
	for (i = 0; i < final_result_count; i++) {
		pat_ptr = final_results[i].pat_ptr;
		memset(results[i].iResult, 0, MAX_BYTES_PER_RESULT);
		decode_string(pat_ptr, results[i].iResult, encoding_table);
		results[i].iDistance = final_results[i].value;
		if (get_data_segment_active(*(pat_ptr + 1))) {
			results[i].iTypeOfResult = ACTIVE;
		}
		else {
			results[i].iTypeOfResult = PASSIVE;
		}
	}
	return rs;
}

/*
 * Reranks the raw sequence of results obtained from the recognizer's
 * recognition process. Two primary modifcations are carried out:
 *
 * 1. Strings with identical recognition distance are sorted so that higher-ranking
 * strings have precedence.
 * 2. Strings are partitioned into active and passive sets, where only an active string
 * can be the first result (unless all results consist of passive strings).
 *
 * There are two different rank strategies available:
 *
 * 1. RANK_ACTIVEPASSIVE_PARTITIONED
 * 
 * This rank method partitions the results into different sequences. The first sequence
 * contains only active strings. The second sequence contains only passive strings. The
 * final results ranking begins with the active sequence and ends with the passive sequence.
 *
 * 2. RANK_ACTIVEPASSIVE_SHIFTED
 *
 * This rank method ensures that the first (highest ranking) result is always an active string,
 * assuming such a string exists in the final results sequence. If a passive string occupies the
 * first position (initial highest rank), the passive string is moved to second position. The first
 * active string is moved up to first place, and all intermediate string results (regardless whether
 * they are active or passive) are shifted to accomodate.
 *
 * *filter - a recognizer object that contains a recognition result
 * rank_method - the rank method to use
 */
static void rerank_results(Filter *aFilter, RankMethod rank_method) {
	switch (rank_method) {
		case RANK_ACTIVEPASSIVE_PARTITIONED:
			rerank_results_partitioned(aFilter);
			break;
		case RANK_ACTIVEPASSIVE_SHIFTED:
			rerank_results_shifted(aFilter);
			break;
	}
}

/*
 * Reranks the raw sequence of results obtained from the recognizer's
 * recognition process. Two primary modifcations are carried out:
 *
 * 1. Strings with identical recognition distance are sorted so that higher-ranking
 * strings have precedence.
 * 2. Strings are partitioned into active and passive sets, where only an active string
 * can be the first result (unless all results consist of passive strings).
 *
 * This rank method partitions the results into different sequences. The first sequence
 * contains only active strings. The second sequence contains only passive strings. The
 * final results ranking begins with the active sequence and ends with the passive sequence.
 *
 *
 * *filter - a recognizer object that contains a recognition result
 */
static void rerank_results_partitioned(Filter *aFilter)
{
	int32 i, ix;
	Result *results, *p1, *p2, *reranked1, *reranked2;
	int32 result_count, n1, n2;

	results = aFilter->parallel_results;
	p1 = aFilter->partition1;
	p2 = aFilter->partition2;
	reranked1 = aFilter->reranked1;
	reranked2 = aFilter->reranked2;
	result_count = MAX_RESULTS < aFilter->parallel_results_count ? MAX_RESULTS : aFilter->parallel_results_count;
	partition(results, result_count, p1, p2, &n1, &n2);	
	rerank_partition(p1, n1, reranked1);
	rerank_partition(p2, n2, reranked2);
	ix = 0;
	for (i = 0; i < n1; i++) {
		results[ix++] = reranked1[i];
	}
	for (i = 0; i < n2; i++) {
		results[ix++] = reranked2[i];
	}
}

/*
 * Reranks the raw sequence of results obtained from the recognizer's
 * recognition process. Two primary modifcations are carried out:
 *
 * 1. Strings with identical recognition distance are sorted so that higher-ranking
 * strings have precedence.
 * 2. Strings are partitioned into active and passive sets, where only an active string
 * can be the first result (unless all results consist of passive strings).
 *
 * This rank method ensures that the first (highest ranking) result is always an active string,
 * assuming such a string exists in the final results sequence. If a passive string occupies the
 * first position (initial highest rank), the passive string is moved to second position. The first
 * active string is moved up to first place, and all intermediate string results (regardless whether
 * they are active or passive) are shifted to accomodate.
 *
 * *filter - a recognizer object that contains a recognition result
 */
static void rerank_results_shifted(Filter *aFilter)
{
	int32 i, ai;
	Result *results, *reranked1, *reranked2;
	int32 result_count;

	results = aFilter->parallel_results;
	reranked1 = aFilter->reranked1;
	reranked2 = aFilter->reranked2;
	result_count = MAX_RESULTS < aFilter->parallel_results_count ? MAX_RESULTS : aFilter->parallel_results_count;
	rerank_partition(results,result_count, reranked1);
	rerank_partition(results,result_count, reranked2);
	if (result_count > 1) {
		/* If top choice is not active, shift */
		if (!get_data_segment_active(*(reranked1[0].pat_ptr + 1))) {
			/* Search for first active word in set */
			ai = -1;
			for (i = 1; i < result_count; i++) {
				if (get_data_segment_active(*(reranked1[i].pat_ptr + 1))) {
					ai = i;
					break;
				}
			}
			/* If there is an active word proceed (otherwise let things stay) */
			if (ai != -1) {
				reranked2[0] = reranked1[ai];
				reranked2[1] = reranked1[0];
				/* Shift all results up to that WordPoint */
				for (i = 1; i < ai; i++) {
					reranked2[i + 1] = reranked1[i];
				}
			}
		}
	}
	/* Copy over reranked data to results */
	for (i = 0; i < result_count; i++) {
		results[i] = reranked2[i];
	}
}

/*
 * Partitions the results list into two partitions. The first
 * partition contains active words, and the second partition
 * contains passive words.
 *
 * The *p1 and *p2 out buffers must be large enough to each be able
 * to hold the entire *results buffer.
 *
 * *results - WordPointer to a sequence of Result structs
 * result_count - the number of Result structs WordPointed to
 * *p1 - out buffer that holds the active partition in rank order
 * *p2 - out buffer that holds the passive partition in rank order
 * *n1 - out paramter that will be set to the number of active Result structs
 * written to the first partition
 * *n2 - out paramter that will be set to the number of passive Result structs
 * written to the second partition
 */
static void partition(Result *results, int32 result_count, Result *p1, Result *p2, int32 *n1, int32 *n2)
{
	int32 i, i1, i2;

	i1 = 0;
	i2 = 0;
	for (i = 0; i < result_count; i++) {
		if (get_data_segment_active(*(results[i].pat_ptr + 1))) {
			/* Active pattern */
			p1[i1++] = results[i];
		}
		else {
			/* Passive pattern */
			p2[i2++] = results[i];
		}
	}
	*n1 = i1;
	*n2 = i2;
}

/*
 * Sorts a partition in rank order.
 *
 * The *reranked out buffer must be large enough to hold
 *  the entire *partition buffer.
 *
 * *partition - WordPointer to a sequence of Result structs
 * partition_count - the number of Result structs WordPointed to
 * *reranked - WordPointer to destination buffer to hold the
 * reranked sequence of Result structs
 */
static void rerank_partition(Result *partition, int32 partition_count, Result *reranked)
{
	int32 i, j, seg_n, reranked_n;
	float32 old_dist;
	Result segment[MAX_RESULTS];
	Result res;
	
	old_dist = -1.0f;
	seg_n = 0;
	reranked_n = 0;
	for (i = 0; i < partition_count; i++) {
		res = partition[i];
		if (old_dist < 0.0f || fabs(res.value - old_dist) < 0.00000001f) {
			segment[seg_n++] = res;
		}
		else {
			if (seg_n > 0) {
				if (seg_n > 1) {
					rerank_segment(segment, seg_n);
					for (j = 0; j < seg_n; j++) {
						reranked[reranked_n++] = segment[j];
					}
				}
				else {
					reranked[reranked_n++] = segment[0];
				}
				seg_n = 0;
			}
			segment[seg_n++] = res;
		}
		old_dist = res.value;
	}
	if (seg_n > 0) {
		rerank_segment(segment, seg_n);
		for (j = 0; j < seg_n; j++) {
			reranked[reranked_n++] = segment[j];
		}
	}
}

/*
 * Sorts a segment in rank order.
 *
 * *segment - WordPointer to a sequence of Result structs
 * n - the number of result structs WordPointed to
 */
static void rerank_segment(Result *segment, int32 n)
{
	qsort(segment, n, sizeof(Result), segment_compar);
}
/*
 * Author: Per Ola Kristensson
 * Last Change: 01 Feb 2008
 *
 * Compares two internal Result structs against each other w.r.t. their
 * relative rank. The function is compatible with ANSI C qsort.
 * 
 */
static int segment_compar(const void *o1, const void *o2)
{
	Result *r1, *r2;
	float32 v1, v2;
	r1 = (Result *)o1;
	r2 = (Result *)o2;
	v1 = get_data_segment_rank(*(r1->pat_ptr + 1));
	v2 = get_data_segment_rank(*(r2->pat_ptr + 1));
	if (v1 < v2) {
		return -1;
	}
	else if (v1 > v2) {
		return 1;
	}
	else {
		return 0;
	}
}

static uint8 pack(uint8 *bitpairs, uint8 n)
{
	uint8 v, a;

	v = 0;
	if (n > 0) {
		a = bitpairs[0];
		v|= (uint8)(a << 6);
	}
	if (n > 1) {
		a = bitpairs[1];
		v|= (uint8)(a << 4);
	}
	if (n > 2) {
		a = bitpairs[2];
		v|= (uint8)(a << 2);
	}
	if (n > 3) {
		a = bitpairs[3];
		v|= a;
	}
	return v;
}

static void get_directional_tendency(int16 *pattern, int32 n, uint8 *out_x, uint8 *out_y, Parameters *parameters, uint8 mode)
{
	int16 t_dx, t_dy;
	int16 x0, y0;
	int16 x1, y1, dx, dy;
	int32 i;
	uint8 x_pos, y_pos;
	uint8 x_chg_pt_counter, y_chg_pt_counter;
	uint8 xdt[100], ydt[100];
	uint8 v1, v2;
	Bounds bounds;

	t_dx = (int16)(0.1 * parameters->key_width);
	t_dy = (int16)(0.1 * parameters->key_height);
	if (n < 2) {
		return;
	}
	/* Keeps track of "change WordPoints" in X */
	x_chg_pt_counter = 0;
	/* Keeps track of "change WordPoints" in Y */
	y_chg_pt_counter = 0;
	if (mode == 1) {
		/* Get the centroid (approximative) */
		get_bounds(pattern, n, &bounds);
		x0 = (int16)(bounds.min_x + abs(bounds.max_x - bounds.min_x) * 0.5);
		y0 = (int16)(bounds.min_y + abs(bounds.max_y - bounds.min_y) * 0.5);
		i = 0;
	} else if (mode == 2) {
		x0 = pattern[0];
		y0 = pattern[1];
		i = 1;
    } else {
        x0 = 0;
        y0 = 0;
    }
	/* x/y_pos: 0: undetermined, 1: negative, 2: positive */
	x_pos = 0;
	y_pos = 0;
	while (i < n) {
		/* Fetch next WordPoint */
		x1 = pattern[i];
		y1 = pattern[i + 1];
		dx = x1 - x0;
		dy = y1 - y0;
		/* If we exceeded the threshold in X, check it */
		if (abs(dx) > t_dx) {
			if (dx < 0) {
				if (x_pos == 0 || x_pos == 2) {
					x_pos = 1;
					if (x_chg_pt_counter < 4) {
						xdt[x_chg_pt_counter++] = x_pos;
					}
				}
			}
			else {
				if (x_pos == 0 || x_pos == 1) {
					x_pos = 2;
					if (x_chg_pt_counter < 4) {
						xdt[x_chg_pt_counter++] = x_pos;
					}
				}
			}
		}
		/* If we exceeded the threshold in Y, check it */
		if (abs(dy) > t_dy) {
			if (dy < 0) {
				if (y_pos == 0 || y_pos == 2) {
					y_pos = 1;
					if (y_chg_pt_counter < 4) {
						ydt[y_chg_pt_counter++] = y_pos;
					}
				}
			}
			else {
				if (y_pos == 0 || y_pos == 1) {
					y_pos = 2;
					if (y_chg_pt_counter < 4) {
						ydt[y_chg_pt_counter++] = y_pos;
					}
				}
			}
		}
		i+= 2;
	}
	v1 = pack(xdt, x_chg_pt_counter);
	v2 = pack(ydt, y_chg_pt_counter);
	*out_x = v1;
	*out_y = v2;
}


static void prune_to_heap(Filter *aFilter, int16 *unknown, Index *search_indices, int32 n_search_indices)
{
	int32 i, j, n;
	uint8 *pattern_ptr;
	uint8 stg_sz, pat_len;
	EncodingTable *encoding_table;
	IndexIgnoreList *index_ignore_list;
	Index index;
	static int16 candidate[MAX_CHARS_PER_STRING * 2];
	int16 candidate_resampled[MAX_N_PRUNING_SAMPLE_POINTS * 2];
	Heap *heap;
	float32 d;
	float32 key_width;
	static uint8 key_trace_buf[MAX_N_PRUNING_SAMPLE_POINTS];
	int32 n_key_trace_buf;
	static uint8 stripped_key_trace_buf[MAX_N_PRUNING_SAMPLE_POINTS];
	int32 n_stripped_key_trace_buf;
	static uint8 u_edge_crossings[MAX_N_PRUNING_SAMPLE_POINTS];
	uint8 n_u_edge_crossings;
	BOOL is_qwerty;
	uint8 n_pruning_sample_WordPoints, n_sample_WordPoints;
	uint8 u_pec;
	int16 ul;
	int16 tsl;
	int16 lup;
	int16 ldown;
	float32 lr;
	WordPoint in_direct;
	WordPoint cand_direct;
	int16 min_side, max_side;
	float32 angle;

	min_side = min(aFilter->parameters->key_height, aFilter->parameters->key_width);
	max_side = max(aFilter->parameters->key_height, aFilter->parameters->key_width);
	encoding_table = aFilter->encoding_table;
	index_ignore_list = aFilter->index_ignore_list;
	n_pruning_sample_WordPoints = aFilter->parameters->n_pruning_sample_points;
	n_sample_WordPoints = aFilter->parameters->n_sample_points;
	is_qwerty = aFilter->kbd_type == QWERTY;
	ul = (int16)((float32)pattern_length(unknown, n_pruning_sample_WordPoints) / (float32)aFilter->parameters->key_width);
	get_string_trace(encoding_table, index_ignore_list, unknown, n_pruning_sample_WordPoints * 2, key_trace_buf, &n_key_trace_buf);
	remove_string_trace_duplicates(key_trace_buf, n_key_trace_buf, stripped_key_trace_buf, &n_stripped_key_trace_buf);
	memset(u_edge_crossings, 0, MAX_N_PRUNING_SAMPLE_POINTS);
	get_edge_crossings(aFilter->edge_key_map->edge_codes, stripped_key_trace_buf, n_stripped_key_trace_buf, u_edge_crossings, &n_u_edge_crossings);
	u_pec = pack(u_edge_crossings, 1);
	key_width = aFilter->parameters->key_width;
	heap = aFilter->heap;
	heap_clear(heap);
	lup = max_side;
	ldown = min_side / 2;
	in_direct = start_direction(unknown, n_pruning_sample_WordPoints, (int16)(min_side / 2));

	for (i = 0; i < n_search_indices; i++) {
		/* Get the index */
		index = search_indices[i];
		/* Fetch WordPointer to first pattern addressed in index */
		pattern_ptr = index.ptr;
		if (pattern_ptr == NULL) {
			continue;
		}
		n = index.len;
		if (n == 0) {
			continue;
		}
		for (j = 0; j < n; j++) {
//			c_t++;
			stg_sz = *pattern_ptr;
//			memset(tmp_buf, 0, sizeof(tmp_buf));
//			decode_string(pattern_ptr, tmp_buf, encoding_table);

//-----------check removed word--------------------------
			if(get_data_segment_remove(*(pattern_ptr + 1))){
				pattern_ptr += stg_sz + PATTERN_HEADER_SIZE;
				continue;
			}

//-------------------------------------------------------
			/* Check the encoded edge crossing */
			if (is_qwerty) {
				if (index.pecs[j] != u_pec) {
					pattern_ptr+= stg_sz + PATTERN_HEADER_SIZE;
					continue;
				}
			}

			if (is_qwerty) {
				tsl = index.lengths[j];
				lr = (float32)ul / (float32)tsl;
				if ( ! ((lr >= 0.6 && lr <= 1.7) || (ul - tsl <= lup && tsl - ul <= ldown))) {
					pattern_ptr+= stg_sz + PATTERN_HEADER_SIZE;			
					continue;
				}
			}
			
			pat_len = decode_WordPoints(pattern_ptr, candidate, encoding_table, index_ignore_list);

			cand_direct.x = candidate[2] - candidate[0];
			cand_direct.y = candidate[3] - candidate[1];
			angle = (cand_direct.x * in_direct.x + cand_direct.y * in_direct.y) / 
				distance_noapprox(cand_direct.x, cand_direct.y, 0, 0) / distance_noapprox(in_direct.x, in_direct.y, 0, 0);
			if(cand_direct.x > min_side &&
				angle < 0.707f){
				pattern_ptr += stg_sz + PATTERN_HEADER_SIZE;			
				continue;
			}

//			unfilter++;
	
			/* All filters passed, decode the geometric pattern and do a pattern match */
			//resample_noapprox(candidate, candidate_resampled, pat_len, n_pruning_sample_WordPoints);
			resample(candidate, candidate_resampled, pat_len, n_pruning_sample_WordPoints);
			d = shape_dist(unknown, candidate_resampled, n_pruning_sample_WordPoints * 2);
//			printf("5 str: %s d: %f tsl: %d\n", tmp_buf, d, tsl);
			heap_insert(heap, d, pattern_ptr);
			pattern_ptr+= stg_sz + PATTERN_HEADER_SIZE;
		}
	}
//	printf("c_e: %d c_d: %d c_o: %d c_f: %d total: %d pruned: %d left: %d\n", c_e, c_d, c_o, c_f, c_t, c_e + c_d + c_o + c_f, c_t - (c_e + c_d + c_o + c_f));
}

static float32 shape_dist(int16 *pat1, int16 *pat2, register int32 n)
{
	register int32 i;
	float32 td;
	
	td = 0;
	for (i = 0; i < n; i++) {
		//td+= distance(pat1[i], pat1[i + 1], pat2[i], pat2[i + 1]);
		td += (pat1[i] - pat2[i]) * (pat1[i] - pat2[i]);
	}
	return td / (n / 2);
}

static void fuzzy_key_search(Parameters *parameters, uint8 *stg, int32 pattern_count, EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 *unknown, Result *out_results, int32 *out_results_count)
{
	int32 i, pat_len;
	uint8 *stg_ptr;
	int32 sz;
	uint8 pat[MAX_CHARS_PER_LEXICON_LINE];
	BOOL match;
	uint8 rank;
	KeyTraceMatch matches[100];
	KeyTraceMatch result;
	uint8 key_trace_buf[MAX_N_SAMPLE_POINTS];
	int32 n_key_trace_buf;
	uint8 stripped_key_trace_buf[MAX_N_SAMPLE_POINTS];
	int32 n_stripped_key_trace_buf;
	int32 match_count;
	uint8 n_pruning_sample_WordPoints, n_sample_WordPoints;

	match_count = 0;
	n_pruning_sample_WordPoints = parameters->n_pruning_sample_points;
	n_sample_WordPoints = parameters->n_sample_points;
	get_string_trace(encoding_table, index_ignore_list, unknown, n_sample_WordPoints * 2, key_trace_buf, &n_key_trace_buf);
	remove_string_trace_duplicates(key_trace_buf, n_key_trace_buf, stripped_key_trace_buf, &n_stripped_key_trace_buf);
	stg_ptr = stg;
	/* Iterate over the entire storage area */
	for (i = 0; i < pattern_count; i++) {
		sz = *stg_ptr;
		/* Get the rank in [0,63], 63 being the top ranked word */
		rank = 63 - get_data_segment_rank(*(stg_ptr + 1));
		pat_len = decode_indices(stg_ptr, pat, encoding_table, index_ignore_list);
		match = word_inside_trace(pat, pat_len, stripped_key_trace_buf, n_stripped_key_trace_buf);
		if (match && match_count < 100) {
			result.ptr = stg_ptr;
			result.rank = rank;
			result.word_len = pat_len;
			matches[match_count++] = result;
		}
		/* Go to next pattern */
		stg_ptr+= sz + PATTERN_HEADER_SIZE;
	}
	qsort(matches, match_count, sizeof(KeyTraceMatch), key_trace_match_compar);
	for (i = 0; i < match_count; i++) {
		out_results[i].value = matches[i].rank;
		out_results[i].pat_ptr = matches[i].ptr;
	}
	*out_results_count = match_count;
}

static int key_trace_match_compar(const void *o1, const void *o2)
{
	uint8 r1, r2;
	int32 wl1, wl2;
	float32 v1, v2;

	r1 = ((KeyTraceMatch *)o1)->rank;
	r2 = ((KeyTraceMatch *)o2)->rank;
	wl1 = ((KeyTraceMatch *)o1)->word_len;
	wl2 = ((KeyTraceMatch *)o2)->word_len;
	v1 = (float32)((1.0 - (1.0 / exp(0.05 * wl1))) * (1.0 - (1.0 / exp(0.01 * r1))));
	v2 = (float32)((1.0 - (1.0 / exp(0.05 * wl2))) * (1.0 - (1.0 / exp(0.01 * r2))));
	if (v1 < v2) {
		return 1;
	}
	else if (v1 > v2) {
		return -1;
	}
	else {
		return 0;
	}
}

static uint8 find_encoded_key(EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 x, int16 y)
{
	int16 *x_components;
	int16 *y_components;
	int32 i, n;
	float32 d, min_d;
	uint8 min_i;

	x_components = encoding_table->x_components;
	y_components = encoding_table->y_components;
	n = encoding_table->n;
	min_i = -1;
	min_d = SHRT_MAX;
	for (i = 0; i < n; i++) {
		if (index_ignore_list->ignore_list[i] == 1) {
			continue;
		}
		d = distance_noapprox(x, y, x_components[i], y_components[i]);
		if (d < min_d) {
			min_d = d;
			min_i = i;
		}
	}
	return min_i;
}

static BOOL word_inside_trace(uint8 *word, int32 n_word, uint8 *trace, int32 n_trace)
{
	int32 i, pos;
	uint8 c;

	/* If both key traces are empty trivially return false */
	if (n_word == 0 || n_trace == 0) {
		return FALSE;
	}
	/* If the trace is shorter than the length of the word,
	 * trivially return false */
	if (n_trace < n_word) {
		return FALSE;
	}
	if (word[0] != trace[0]) {
		return FALSE;
	}
	if (word[n_word - 1] != trace[n_trace - 1]) {
		return FALSE;
	}
//	/* Enforce constraint: The first key in the word must be in the first
//	 * three key positions in the trace */
//	c = word[0];
//	pos = 0;
//	while (c != trace[pos]) {
//		pos++;
//		if (pos > 2 || pos >= n_trace) {
//			return FALSE;
//		}
//	}
//	/* Enforce constraint: The last key in the word must be in the last
//	 * three key positions in the trace */
//	c = word[n_word - 1];
//	pos = n_trace - 1;
//	while (c != trace[pos]) {
//		pos--;
//		if (pos < n_trace - 4 || pos < 0) {
//			return FALSE;
//		}
//	}
	/* The variable "pos" keeps track of the position in the trace */
	pos = 0;
	/* Loop over all keys in the word we are matching against */
	for (i = 0; i < n_word; i++) {
		/* The current key in the word */
		c = word[i];
		/* If the current key in the word does not match the current
		 * key in the trace (indexed by "pos"), then move forward "pos"
		 * until this is the case, or the trace is exhausted without a
		 * match. */
		while (pos < n_trace && trace[pos] != c) {
			pos++;
		}
		/* If "pos" has exhausted the trace the trace cannot contain
		 * the word, return false */
		if (pos >= n_trace) {
			return FALSE;
		}
	}
	/* The trace must contain the word, return true */
	return TRUE;
}

static void get_string_trace(EncodingTable *encoding_table, IndexIgnoreList *index_ignore_list, int16 *pat, int32 n, uint8 *out_trace, int32 *out_n)
{
	register int32 i, j;
	int16 x, y;

	j = 0;
	for (i = 0; i < n; i+= 2) {
		x = pat[i];
		y = pat[i + 1];
		out_trace[j++] = find_encoded_key(encoding_table, index_ignore_list, x, y);
	}
	*out_n = j;
}

static void remove_string_trace_duplicates(uint8 *in_trace, int32 in_n, uint8 *out_trace, int32 *out_n)
{
	register int32 i, j;
	uint8 k1, k2;

	if (in_n < 2) {
		out_trace[0] = in_trace[0];
		*out_n = in_n;
		return;
	}
	k1 = in_trace[0];
	j = 0;
	out_trace[j++] = k1;
	for (i = 1; i < in_n; i++) {
		k2 = in_trace[i];
		if (k1 != k2) {
			out_trace[j++] = k2;
		}
		k1 = k2;
	}
	*out_n = j;
}

static EdgeKeyMap * create_edge_key_map(EdgeMap *edge_map, EncodingTable *encoding_table)
{
	register int32 i;
	int32 n;
	int16 y;
	EdgeKeyMap *edge_key_map;
	int16 edge1, edge2;

	edge1 = edge_map->edge1;
	edge2 = edge_map->edge2;
	edge_key_map = q_malloc( sizeof(EdgeKeyMap));
	if (edge_key_map == NULL) {
		return NULL;
	}
	n = encoding_table->n;
	edge_key_map->n = n;
	edge_key_map->edge_codes = q_malloc( sizeof(uint8) * n);
	if (edge_key_map->edge_codes == NULL) {
		return NULL;
	}
	for (i = 0; i < n; i++) {
		y = encoding_table->y_components[i];
		if (y < edge1) {
			edge_key_map->edge_codes[i] = 0;
		}
		else if (y > edge2) {
			edge_key_map->edge_codes[i] = 2;
		}
		else {
			edge_key_map->edge_codes[i] = 1;
		}
	}
	return edge_key_map;
}

static void get_edge_crossings(uint8 *edge_codes, uint8 *trace, int32 n, uint8 *out_edge_crossings, uint8 *out_n_edge_crossings)
{
	register int32 i;
	int32 j;
	uint8 k1, k2;

	j = 0;
	if (n < 2) {
		return;
	}
	k1 = edge_codes[trace[0]];
	for (i = 1; i < n; i++) {
		k2 = edge_codes[trace[i]];
		/* No edge transition */
		if (k1 == k2) {
			continue;
		}
		/* Crossing edge 1 from above */
		else if (k1 == 0 && k2 == 1) {
			out_edge_crossings[j++] = 1;
		}
		/* Crossing edge 1 and edge 2 from above */
		else if (k1 == 0 && k2 == 2) {
			out_edge_crossings[j++] = 1;
			out_edge_crossings[j++] = 3;
		}
		/* Crossing edge 1 from below*/
		else if (k1 == 1 && k2 == 0) {
			out_edge_crossings[j++] = 0;
		}
		/* Crossing edge 2 from above */
		else if (k1 == 1 && k2 == 2) {
			out_edge_crossings[j++] = 3;
		}
		/* Crossing edge 2 and edge 1 from below */
		else if (k1 == 2 && k2 == 0) {
			out_edge_crossings[j++] = 2;
			out_edge_crossings[j++] = 0;
		}
		/* Crossing edge 2 from below */
		else if (k1 == 2 && k2 == 1) {
			out_edge_crossings[j++] = 2;
		}
		k1 = k2;
	}
	*out_n_edge_crossings = j;
}


static void parallel_channel_integration(Filter *aFilter, int16 *unknown)
{
	register int32 i, n;
	Heap *heap;
	HeapNode *nodes;
	int32 results_count;
	EncodingTable *encoding_table;
	IndexIgnoreList *index_ignore_list;
	int16 candidate[MAX_CHARS_PER_STRING * 2];
	int16 candidate_resampled[MAX_N_SAMPLE_POINTS * 2];
	uint8 *storage;
	uint8 *pattern_ptr;
	uint8 pat_len;
	float32 ld;
	float32 *probabilities;
	Result *results;
	float32 lp;
	int32 offset;
	float32 s_sigma, l_sigma;
	uint8 rank;
	uint8 n_sample_WordPoints;
	int16 in_corner[2 * MAX_CHARS_PER_STRING];
	int32 in_corner_num;
	int16 cand_corner[2 * MAX_CHARS_PER_STRING];
	int32 cand_corner_num;
	int16 key_width, key_height;

//	int flag = 1;

#ifdef FILTER_LOGGER
	FILE *log_stream;
#endif

	n_sample_WordPoints = aFilter->parameters->n_pruning_sample_points;
	encoding_table = aFilter->encoding_table;
	index_ignore_list = aFilter->index_ignore_list;
	storage = aFilter->storage;
	results = aFilter->parallel_results;
	key_width = aFilter->parameters->key_width;
	key_height = aFilter->parameters->key_height;
	results_count = 0;
	memset(results, 0, (HEAP_CAPACITY + 1) * sizeof(Result));
	heap = aFilter->heap;
	n = heap->n + 1;
	nodes = heap->nodes;
	probabilities = aFilter->probabilities;
	memset(probabilities, 0, HEAP_CAPACITY * 2 * sizeof(float32));
	offset = HEAP_CAPACITY;
	s_sigma = aFilter->shape_sigma;
	l_sigma = aFilter->location_sigma;

	
	get_corner_from_signal(aFilter, unknown, n_sample_WordPoints, in_corner, &in_corner_num);

	for (i = 1; i < n; i++) {
		pattern_ptr = nodes[i].ptr;
		rank = 63 - get_data_segment_rank(*(pattern_ptr + 1));
		pat_len = decode_WordPoints(pattern_ptr, candidate, encoding_table, index_ignore_list);

		/* Resample the candidate into an equidistant N-WordPoint pattern */
		resample_noapprox(candidate, candidate_resampled, pat_len, n_sample_WordPoints);





		get_corner_from_signal(aFilter, candidate, pat_len, cand_corner, &cand_corner_num);
		
//		shape_location_dist(unknown, candidate_resampled, unk_center_x, unk_center_y, cand_center_x, cand_center_y, s, n_sample_WordPoints * 2, &sd, &ld);
		ld = distance_array(unknown, candidate_resampled, n_sample_WordPoints * 2) / n_sample_WordPoints;
		ld += 2 * edit_distance(in_corner, in_corner_num, cand_corner, cand_corner_num, key_width, key_height);

//		sd*= (float32)(1.0f / exp(0.001 * rank));
		ld*= (float32)(1.0f / exp(0.001 * rank));

//		sp = gauss_prob(sd, s_sigma);
		lp = gauss_prob(ld, l_sigma);

//		printf("str: %s sd: %f ld: %f sp: %f lp: %f\n", tmp_buf, sd, ld, sp, lp);
		probabilities[i - 1] = 1;
		probabilities[i - 1 + offset] = lp;
	}
#ifdef FILTER_LOGGER
	log_stream = log_open();
	log_probabilities(log_stream, probabilities, HEAP_CAPACITY * 2, aFilter->encoding_table, heap->nodes);
	log_close(log_stream);
#endif
	integrate(probabilities, HEAP_CAPACITY * 2);
	n-= 1;
	for (i = 0; i < n; i++) {
		results[i].value = probabilities[i];
		results[i].pat_ptr = nodes[i + 1].ptr;
	}
	results_count = n;
	/* Sort results */
	qsort(results, results_count, sizeof(Result), result_compar);
	aFilter->parallel_results_count = results_count;
}

static BOOL edge_match(uint8 *edge_crossings1, uint8 n1, uint8 *edge_crossings2, uint8 n2)
{
	register int32 i;

	if (n1 != n2) {
		return FALSE;
	}
	for (i = 0; i < n1; i++) {
		if (edge_crossings1[i] != edge_crossings2[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * Compares two internal Result structs against each other w.r.t. their
 * values (confidence scores). The function is compatible with ANSI C qsort.
 */
static int result_compar(const void *o1, const void *o2)
{
	float32 v1, v2;
	v1 = ((Result *)o1)->value;
	v2 = ((Result *)o2)->value;
	if (v1 < v2) {
		return 1;
	}
	else if (v1 > v2) {
		return -1;
	}
	else {
		return 0;
	}
}

/*
 * Integrates two sets of probabilites, one from the shape
 * channel, and the other from the location channel, using
 * Bayes' rule and subsequent marginalization.
 *
 * *probabilities - a contiguous segment of memory
 * n 
 *
 */
static void integrate(float32 *probabilities, int32 n)
{
	register int32 i;
	int32 offset;

	offset = n / 2;
	/* Bayes' rule */
	for (i = 0; i < offset; i++) {
		// 1. Location, 2. Shape
		probabilities[i] = probabilities[i] * probabilities[i + offset];
	}
}


static void shape_location_dist(int16 *unknown, int16 *candidate, int16 ucx, int16 ucy, int16 ccx, int16 ccy, float32 s, register int32 n, float32 *sd, float32 *ld)
{
	register int32 i;
	float32 a;
	//float32 b;
	float32 sd0;
	float32 tsd;
	//float32 ld0;
	float32 tld;
	int16 x1, y1, x2, y2;
	
	tsd = 0.0f;
	tld = 0.0f;
	for (i = 0; i < n; i+= 2) {
		x1 = unknown[i];
		y1 = unknown[i + 1];
		x2 = candidate[i];
		y2 = candidate[i + 1];
		/* Location channel */
		a = distance(x1, y1, x2, y2);
		//b = bi_alpha((float32)(i / 2), (float32)n, 0.5f);
		//ld0 = a * 1.0f;
		//ld0 = a * b;
		//tld+= ld0;
		tld+= a;
		/* Shape channel */
		x1-= ucx;
		y1-= ucy;
		x2-= ccx;
		y2-= ccy;
		//x2*= s;
		//y2*= s;
		x1 = (int16)(x1 * s);
		y1 = (int16)(y1 * s);
		sd0 = distance(x1, y1, x2, y2);
		tsd+= sd0;
	}
	*sd = tsd / (float32)(n / 2);
	*ld = tld / (float32)(n / 2);
}

static int16 center_x(Bounds *bounds)
{
	return bounds->min_x + ((bounds->max_x - bounds->min_x) / 2);
}

static int16 center_y(Bounds *bounds)
{
	return bounds->min_y + ((bounds->max_y - bounds->min_y) / 2);
}

static void get_bounds(int16 *pat, int32 n, Bounds *bounds)
{
	int i;
	int16 min_x, min_y, max_x, max_y, x, y;

	min_x = SHRT_MAX;
	min_y = SHRT_MAX;
	max_x = SHRT_MIN;
	max_y = SHRT_MIN;
	for (i = 0; i < n; i+= 2) {
		x = pat[i];
		y = pat[i + 1];
		if (x < min_x) {
			min_x = x;
		}
		if (y < min_y) {
			min_y = y;
		}
		if (x > max_x) {
			max_x = x;
		}
		if (y > max_y) {
			max_y = y;
		}
	}
	bounds->min_x = min_x;
	bounds->min_y = min_y;
	bounds->max_x = max_x;
	bounds->max_y = max_y;
}

/*
 * Calculates a probability from a given sample distance
 * using the Gaussian probability density function.
 *
 * d is the distance
 * sigma is the standard deviation in the Gaussian
 *
 * The return value is a probability sampled from the Gaussian
 *
 */
static float32 gauss_prob(float32 d, float32 sigma)
{
	return (float32)(exp(-0.025f * pow(d / sigma, 2.0f)));
}

/*
 * Calculates a bi-linear weighted value.
 *
 * i the sample index
 * n the total number of samples
 * the weight in [0,1] for the mid section (i.e. n / 2)
 *
 * The return value is a weighted value.
 *
 */
static float32 bi_alpha(float32 i, float32 n, float32 w)
{
	float32 n2, x;
	
	n2 = 0.5f * n;
	x = i / n2;
	if (i < n2) {
		return 1.0f - w * x;
	}
	else {
		return w * x;
	}
}

/*
 * A non-approximate but correct Euclidean distance implementation
 * (within the range of int16s. This function needs to be called rather
 * than the approximative in boundary cases.
 */
static float32 distance_noapprox(float32 x1, float32 y1, float32 x2, float32 y2)
{
	float32 dx, dy;

	dx = x1 - x2;
	dy = y1 - y2;
	return sqrtf(dx * dx + dy * dy);
}

/*
 * Calculates the approximate Euclidean distance between
 * (x1,y1) and (x2,y2). This code is from the book
 * Graphics Gems, Academic Press, 1990. The license allows
 * commerical use without attribution.
 *
 * The calculation will be exact or an overestimation.
 */
static int16 distance(int16 x1, int16 y1, int16 x2, int16 y2)
{
	if ((x2 -= x1) < 0) {
		x2 = -x2;
	}
	if ((y2 -= y1) < 0) {
		y2 = -y2;
	}
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1) );
}


static float32 distance_array(int16 *p, int16 *q, int32 n)
{
	float32 dist = 0;
	int32 i, m;
	
	m = n / 2;
	for(i = 0; i < m; i += 2){
		dist += 1.1f * distance(*p, *(p+1), *q, *(q+1));
		p += 2;
		q += 2;
	}
	for(i = m; i < n; i += 2){
		dist += 0.9f * distance(*p, *(p+1), *q, *(q+1));
		p += 2;
		q += 2;
	}
	return dist;
}

/*
 * Calculates the total spatial length of a pattern.
 *
 * *pat is a sequence of (x,y) WordPoints
 * n is the number of WordPoints in *pat (this number must be even)
 *
 * The return value is total spatial (Euclidean) length of the
 * input pattern sequence. This number may be approximate, in which
 * case it is never an underestimation (only an exact result or an
 * overestimation can occur).
 *
 */
static int16 pattern_length(int16 *pat, int32 n)
{
    int16 l;
    int32 i;
	int16 *x1, *y1, *x2, *y2;

	l = 0;
    if (n > 1) {
		x1 = pat;
		y1 = pat + 1;
		x2 = pat + 2;
		y2 = pat + 3;
		for (i = 1; i < n; i++) {
			l += distance(*x1, *y1, *x2, *y2);
			x1 += 2;
			x2 += 2;
			y1 += 2;
			y2 += 2;
		}
		return l;
    }
    else {
		return 0;
    }
}

static float32 pattern_length_noapprox(int16 *pat, int32 n)
{
    float32 l;
    int32 i;
	int16 *x1, *y1, *x2, *y2;

	l = 0;
    if (n > 1) {
		x1 = pat;
		y1 = pat + 1;
		x2 = pat + 2;
		y2 = pat + 3;
		for (i = 1; i < n; i++) {
			l += distance_noapprox(*x1, *y1, *x2, *y2);
			x1 += 2;
			x2 += 2;
			y1 += 2;
			y2 += 2;
		}
		return l;
    }
    else {
		return 0;
    }
}

static uint8 * get_pattern_ptr_at_index(Filter *aFilter, Index *index, const char *str)
{
	int32 i, n;
	uint8 len;
	uint8 *stg;
	EncodingTable *encoding_table;
	char buf[MAX_BYTES_PER_STRING];

	encoding_table = aFilter->encoding_table;
	n = index->len;
	stg = index->ptr;
	/* Exhaustive search through the patterns at the index */
	for (i = 0; i < n; i++) {
		len = stg[0];
		memset(buf, 0, MAX_BYTES_PER_STRING);
		decode_string(stg, buf, encoding_table);
		if (!get_data_segment_remove(*(stg + 1)) && utf8_strncmp(buf, str, MAX_BYTES_PER_STRING) == 0) {
			return stg;
		}
		stg+= len + PATTERN_HEADER_SIZE;
	}
	return NULL;
}

/*
 * Returns the WordPointer to the pattern in the recognizer
 * that corresponds to the given string.
 *
 * *filter is a recognizer object
 * *str is a string
 *
 * The return value is a WordPointer to the pattern in the recognizer,
 * or NULL if no pattern corresponds to the given string.
 *
 */
static uint8 * get_pattern_ptr(Filter *aFilter, const char *str)
{
	int32 m, l;
	int32 ii1, ii2;
	uint8 i1, i2;
	Index index;
	uint8 *pat_ptr;
	EncodingTable *encoding_table;

	encoding_table = aFilter->encoding_table;
	m = encoding_table->n;
	l = utf8_strlen(str);
	/* A string with less than 2 characters cannot
	 * match any string in recognizer storage
	 */
	if (l < 2) {
		return NULL;
	}
	/* Find the index that indexes patterns that begin and end with
	 * the same letters as the query string
	 */
	ii1 = get_str_encoding_index(encoding_table, utf8_char_at(str, 0));
	if (ii1 < 0) {
		return NULL;
	}
	ii2 = get_str_encoding_index(encoding_table, utf8_char_at(str, l - 1));
	if (ii2 < 0) {
		return NULL;
	}
	i1 = ii1;
	i2 = ii2;
	index = aFilter->indices[i1 * encoding_table->n + i2];
	pat_ptr = get_pattern_ptr_at_index(aFilter, &index, str);
	if (pat_ptr == NULL) {
		/* Try extra indices */
		if (aFilter->extra_indices != NULL) {
			index = aFilter->extra_indices[i1 * encoding_table->n + i2];
			pat_ptr = get_pattern_ptr_at_index(aFilter, &index, str);
		}
	}
	return pat_ptr;
}

static uint8 * get_system_pattern_ptr(Filter *aFilter, const char *str)
{
	int32 m, l;
	int32 ii1, ii2;
	uint8 i1, i2;
	Index index;
	uint8 *pat_ptr;
	EncodingTable *encoding_table;

	encoding_table = aFilter->encoding_table;
	m = encoding_table->n;
	l = utf8_strlen(str);
	/* A string with less than 2 characters cannot
	 * match any string in recognizer storage
	 */
	if (l < 2) {
		return NULL;
	}
	/* Find the index that indexes patterns that begin and end with
	 * the same letters as the query string
	 */
	ii1 = get_str_encoding_index(encoding_table, utf8_char_at(str, 0));
	if (ii1 < 0) {
		return NULL;
	}
	ii2 = get_str_encoding_index(encoding_table, utf8_char_at(str, l - 1));
	if (ii2 < 0) {
		return NULL;
	}
	i1 = ii1;
	i2 = ii2;
	index = aFilter->indices[i1 * encoding_table->n + i2];
	pat_ptr = get_pattern_ptr_at_index(aFilter, &index, str);
	return pat_ptr;
}

static uint8 * get_extra_pattern_ptr(Filter *aFilter, const char *str)
{
	int32 m, l;
	int32 ii1, ii2;
	uint8 i1, i2;
	Index index;
	EncodingTable *encoding_table;

	encoding_table = aFilter->encoding_table;
	m = encoding_table->n;
	l = utf8_strlen(str);
	/* A string with less than 2 characters cannot
	 * match any string in recognizer storage
	 */
	if (l < 2) {
		return NULL;
	}
	/* Find the index that indexes patterns that begin and end with
	 * the same letters as the query string
	 */
	ii1 = get_str_encoding_index(encoding_table, utf8_char_at(str, 0));
	if (ii1 < 0) {
		return NULL;
	}
	ii2 = get_str_encoding_index(encoding_table, utf8_char_at(str, l - 1));
	if (ii2 < 0) {
		return NULL;
	}
	i1 = ii1;
	i2 = ii2;

	/* Try extra indices */
	if (aFilter->extra_indices != NULL) {
		index = aFilter->extra_indices[i1 * encoding_table->n + i2];
		return get_pattern_ptr_at_index(aFilter, &index, str);
	}
	return NULL;
}


static InternalFilterResult get_decoding_storage_byte_count(uint8 *storage, int32 pattern_count, EncodingTable *encoding_table, int32 *out_byte_count)
{
	register int32 i;
	uint8 sz;
	uint8 *ptr;
	char buf[MAX_BYTES_PER_LEXICON_LINE];
	int32 byte_count;

	byte_count = 0;
	ptr = storage;
	for (i = 0; i < pattern_count; i++) {
		sz = *ptr;
		memset(buf, 0, MAX_BYTES_PER_LEXICON_LINE);
		decode_string(ptr, buf, encoding_table);
		byte_count+= strlen(buf) + 1;
		ptr+= PATTERN_HEADER_SIZE + sz;
	}
	*out_byte_count = byte_count;
	return FILTER_SUCCESS;
}

static InternalFilterResult decode_storage(uint8 *storage, int32 pattern_count, EncodingTable *encoding_table, char *out_buffer, int32 *out_string_count)
{
	register int32 i;
	uint8 sz;
	uint8 *ptr;
	char *out_buf_ptr;
	char buf[MAX_BYTES_PER_LEXICON_LINE];
	int32 byte_count;

	ptr = storage;
	out_buf_ptr = out_buffer;
	for (i = 0; i < pattern_count; i++) {
		sz = *ptr;
		memset(buf, 0, MAX_BYTES_PER_LEXICON_LINE);
		decode_string(ptr, buf, encoding_table);
		byte_count = strlen(buf) + 1;
		memcpy(out_buf_ptr, buf, byte_count);
		out_buf_ptr+= byte_count;
		ptr+= PATTERN_HEADER_SIZE + sz;
	}
	*out_string_count = pattern_count;
	return FILTER_SUCCESS;
}

static BOOL add_extra_strings(Filter *aFilter, const char **strs, int32 strs_n)
{
	register int32 i;
	int32 bc;
	uint8 *tmp_pattern_stg;
	int32 byte_count;
	MemoryDataBuffer mdb;
	InternalFilterResult result;
	int32 pattern_count;
	int32 char_byte_count;
	char *strs_buffer, *strs_buffer_ptr;
	int32 string_count;

	/* Count the number of bytes required to store the strings we are going to add
	 * to a temporary string buffer. */
	bc = 0;
	for (i = 0; i < strs_n; i++) {
		bc+= strlen(strs[i]) + 1;
	}
	if (bc == 0) {
		return TRUE;
	}
	/* Count the number of bytes required to store the strings decoded from extra 
	 * storage into a temporary buffer */
	if (aFilter->extra_storage != NULL) {
		get_decoding_storage_byte_count(aFilter->extra_storage, aFilter->extra_pattern_count, aFilter->encoding_table, &char_byte_count);
	}
	else {
		char_byte_count = 0;
	}
	/* Allocate the temporary string buffer to hold the decoded strings from
	 * extra storage, and add the bytes necessary to hold. */
	strs_buffer = q_malloc( sizeof(char) * (char_byte_count + bc));
	if (aFilter->extra_storage != NULL) {
		decode_storage(aFilter->extra_storage, aFilter->extra_pattern_count, aFilter->encoding_table, strs_buffer, &string_count);
	}
	else {
		string_count = 0;
	}
	/* Add the strings we are going to add to the temporary string buffer */
	strs_buffer_ptr = strs_buffer;
	/* Forward WordPointer to the end of the block where strings were decoded from extra
	 * storage */
	strs_buffer_ptr+= char_byte_count;
	/* Copy over all the strings we are going to add to the end of the temporary
	 * memory buffer that holds the decoded strings from extra storage.
	 */
	for (i = 0; i < strs_n; i++) {
		bc = strlen(strs[i]) + 1;
		memcpy(strs_buffer_ptr, strs[i], bc);
		strs_buffer_ptr+= bc;
	}
	strs_buffer_ptr = strs_buffer;
	mdb.block_begin = strs_buffer;
	mdb.current_line_count = 0;
	mdb.pointer = strs_buffer_ptr;
	mdb.total_line_count = string_count + strs_n;
	/* Get the byte count required to form new extra storage */
	result = get_data_buffer_encoding_byte_count(&mdb, memory_stream_data_buffer_getline, aFilter->encoding_table, &byte_count, &string_count);
	/* Rewind the data stream to the same stream position as before the previous read */
	mdb.block_begin = strs_buffer;
	mdb.current_line_count = 0;
	mdb.pointer = strs_buffer_ptr;
	/* Create temporary space for extra storage */
	tmp_pattern_stg = q_malloc( sizeof(uint8) * byte_count);
	/* Create the new extra storage */
	result = create_patterns_from_data_buffer(&mdb, memory_stream_data_buffer_getline, tmp_pattern_stg, byte_count, aFilter->encoding_table, string_count, &pattern_count, TRUE);
	/* Free the old extra storage memory area */
	if (aFilter->extra_storage != NULL) {
		q_free(aFilter->extra_storage);
	}
	/* Allocate a new one to fit the new data */
	aFilter->extra_storage = q_malloc( byte_count);
	aFilter->extra_storage_size = byte_count;
	aFilter->extra_pattern_count = pattern_count;
	memset(aFilter->extra_storage, 0, byte_count);

	if (aFilter->extra_index_count > 0) {
		for (i = 0; i < aFilter->extra_index_count; i++) {
			q_free(aFilter->extra_indices[i].lengths);
			q_free(aFilter->extra_indices[i].pecs);
			q_free(aFilter->extra_indices[i].xdts);
			q_free(aFilter->extra_indices[i].ydts);
			q_free(aFilter->extra_indices[i].x0dts);
			q_free(aFilter->extra_indices[i].y0dts);
			aFilter->extra_indices[i].len = 0;
			aFilter->extra_indices[i].ptr = NULL;
		}
		q_free(aFilter->extra_indices);
		aFilter->extra_indices = NULL;
	}

	
	result = create_indices(tmp_pattern_stg, aFilter->extra_storage, aFilter->extra_pattern_count, aFilter->encoding_table->n, &(aFilter->extra_indices), &(aFilter->extra_index_count));
	result = compute_index_values(aFilter->extra_indices, aFilter->extra_index_count, aFilter->encoding_table, aFilter->index_ignore_list, aFilter->edge_key_map, aFilter->parameters);
	q_free(strs_buffer);
	q_free(tmp_pattern_stg);
	return TRUE;
}

BOOL addWord(Filter* aFilter, const char* aWord)
{
	int n;
	char **strs;
	BOOL rv;

	strs = q_malloc( sizeof(char *));
	n = strlen(aWord) + 1;
	strs[0] = q_malloc( n * sizeof(char));
	strcpy(strs[0], aWord);
	rv = add_extra_strings(aFilter, (const char **)strs, 1);
	q_free(strs[0]);
	q_free(strs);
	return rv;
}

BOOL addWords(Filter* aFilter, const char** aWords, int32 aNumOfWords)
{
	return add_extra_strings(aFilter, aWords, aNumOfWords);
}

BOOL wordExisted(Filter* aFilter, const char* aWord)
{
	return get_pattern_ptr(aFilter, aWord) != NULL;
}

BOOL extraWordExisted(Filter* aFilter, const char* aExtraWord)
{
	return get_extra_pattern_ptr(aFilter, aExtraWord) != NULL;
}

BOOL removeWord(Filter* aFilter, const char* aWord)
{
	uint8 *ptr;

	ptr = get_extra_pattern_ptr(aFilter, aWord);
	if (ptr == NULL){
		return FALSE;
	}
	set_data_segment_remove(ptr + 1, TRUE);
	return TRUE;
}

BOOL isActiveWord(Filter* aFilter, const char* aWord)
{
	uint8 *ptr;

	ptr = get_pattern_ptr(aFilter, aWord);
	if (ptr == NULL) {
		return FALSE;
	}
	return get_data_segment_active(*(ptr + 1));
}

BOOL setWordActive(Filter* aFilter, const char* aWord, BOOL aActive)
{
	uint8 *ptr;

	ptr = get_pattern_ptr(aFilter, aWord);
	if (ptr == NULL) {
		return FALSE;
	}
	set_data_segment_active(ptr + 1, aActive);
	return TRUE;
}

BOOL setAllActive(Filter* aFilter, BOOL aActive)
{
	int32 i, n;
	uint8 len;
	uint8 *stg;
    if (aFilter == NULL) return FALSE;

	stg = aFilter->storage;
	n = aFilter->pattern_count;
	for (i = 0; i < n; i++) {
		len = stg[0];
		set_data_segment_active(stg + 1, aActive);
		stg+= len + PATTERN_HEADER_SIZE;
	}
	return TRUE;
}

void addFilterInitCallback(Filter* aFilter, void* aListener, filterInitCallback aFilterInitCallback)
{
    if (aFilter == NULL) return;
	aFilter->init_callback = aFilterInitCallback;
	aFilter->init_callback_listener = aListener;
}

void removeFilterInitCallback(Filter* aFilter)
{
    if (aFilter == NULL) return;
	aFilter->init_callback = NULL;
	aFilter->init_callback_listener = NULL;
}

static void resample_noapprox(int16 *in, int16 *out, int32 n, int32 pts)
{
	int32 i;
	int16 frontX, frontY, curX, curY, segNum, j, obj;
	float32 dx, dy, total, rest, segLength;

	segLength = ((float32)pattern_length_noapprox(in, n)) / (pts - 1);
	out[0] = in[0];
	out[1] = in[1];
	frontX = in[0];
	frontY = in[1];
	rest = 0;
	obj = 2;

	for(i = 1; i < n; i++){
		curX = in[2 * i];
		curY = in[2 * i + 1];
		total = distance_noapprox(frontX, frontY, curX, curY);
		dx = (curX - frontX) / total;
		dy = (curY - frontY) / total;
		segNum = (int16)((total + rest) / segLength);
		for(j = 1; j <= segNum; j++){
			out[obj++] = (int16)(frontX + dx * (j * segLength - rest) + 0.5);
			out[obj++] = (int16)(frontY + dy * (j * segLength - rest) + 0.5);
		}
		rest += total - segNum * segLength;
		frontX = curX;
		frontY = curY;
	}
	out[2 * pts - 2] = in[2 * n - 2];
	out[2 * pts - 1] = in[2 * n - 1];
}

static void resample(int16 *in, int16 *out, int32 n, int32 pts)
{
	static int16 segment_buf[MAX_SEGMENTS];

    float32 l, segl, dx, dy;
    int16 x1, y1, x2, y2;
    int32 i, m, a, ptsseg, j, out_max, end;

	m = n * 2;
    /* Get the spatial length of the WordPoint sequence */
    l = pattern_length(in, n);
    /* Compute the segment length for the given length and number of
     * sample WordPoints */
    segl = l / (pts - 1);
	memset(segment_buf, -1, MAX_SEGMENTS * 2);
    pts_per_seg(in, n, segl, segment_buf);

    x1 = in[0];
	y1 = in[1];
    /* The a variable keeps track of the interpolated WordPoint index */
    a = 0;
	out_max = pts * 2;
    for (i = 2; i < m; i+= 2) {
		x2 = in[i];
		y2 = in[i + 1];
		/* Number of WordPoints to interpolate into this segment */
		ptsseg = segment_buf[(i / 2) - 1];
		/* Find the dx and dy delta distances for each WordPoint to be
		* interpolated */
		dx = -1.0f;
		dy = -1.0f;
		if (ptsseg - 1 <= 0) {
			dx = 0.0f;
			dy = 0.0f;
		}
		else {
			dx = (x2 - x1) / (float32)(ptsseg);
			dy = (y2 - y1) / (float32)(ptsseg);
		}
		/* Only try to interplate if at least one WordPoint should be interpolated
		 * into this segment */
		if (ptsseg > 0) {
			/* Iterate over all WordPoints to be interpolated into this segment */
			for (j = 0; j < ptsseg; j++) {
				if (j == 0) {
					/* If we are at the first WordPoint, apply the rest segment
					* length */
					if (a < out_max) {
						out[a] = x1;
						out[a + 1] = y1;
						a+= 2;
					}
				}
				else {
					/* For all other WordPoints to be interpolated, calculate
					* x and y delta */
					if (a < out_max) {
						out[a] = (int16)(x1 + j * dx);
						out[a + 1] = (int16)(y1 + j * dy);
						a+= 2;
					}
				}
			}
		}
		x1 = x2;
		y1 = y2;
	}
	/* Check that all WordPoints that should be interpolated actually are. This may
	 * not always be true due to the approximate nature of interpolation.
	 *
	 * Notes: The variable "a" counts x and y components separately and is always even,
	 * pts * 2 is always even, 1 WordPoint (end WordPoint) is reserved to the very end
	 */
	end = (pts * 2) - 2;
	if (a < end) {
		/* Linearly interpolate missing WordPoints between last inserted WordPoint and end WordPoint */
		for (i = a; i < end; i+= 2) {
			out[i] = (out[i - 2] + in[m - 2]) / 2;
			out[i + 1] = (out[i - 1] + in[m - 1]) / 2;
		}
	}
	/* Set the last sample WordPoint to the identical corresponding sample WordPoint in
	 * the input pattern. The end WordPoint and start WordPoint are the only WordPoints that
	 * are not computed and that are guaranteed to be identical to the input
	 * patterns' corresponding WordPoints.
	 */
	out[out_max - 2] = in[m - 2];
	out[out_max - 1] = in[m - 1];
}

/*
 * Calculates the number of sample WordPoints that needs to be interpolated
 * onto each line segment (e.g. this function is only concerned with
 * up-sampling).
 *
 * The function assumes the pattern is intended to be resampled with
 * equidistant spacing. The parametr segl (segment length) must be
 * provided as the "split criterion".
 *
 * *pat is a sequence of (x,y) WordPoints
 * n is the number of WordPoints in *pat
 * segl is the intended equidistant segment length
 * *segment_buf is an output buffer that this function will fill
 * in with the number of sample WordPoints required into each segment.
 * The segment buffer must have space for (n - 1) integers.
 *
 * The return value is the "rest segment" length , that is a segment that
 * overflowed at the the end but was still less than the segment length.
 *
 */
static float32 pts_per_seg(int16 *pat, int32 n, float32 segl, int16 *segment_buf)
{
	int32 i, m;
	int16 x1, y1, x2, y2, ps;
    float32 restsegl, cursegl;

	m = n * 2;
    restsegl = 0.0f;
	x1 = pat[0];
	y1 = pat[1];
    for (i = 2; i < m; i+= 2) {
		x2 = pat[i];
		y2 = pat[i + 1];
		/* Get the length of the current segment */
		cursegl = distance(x1, y1, x2, y2);
		/* Add the rest of the segment length */
		cursegl+= restsegl;
		restsegl = 0.0f;
		/* Find how many WordPoints that can be interpolated into this segment */
		ps = (int16)((cursegl / segl));
		if (ps == 0) {
			/* No WordPoints for this segment, add the current segment length
			* to the rest segment */
			restsegl+= cursegl;
		}
		else {
			/* Find the remaining segment length and add it to the rest
			* segment */
			restsegl+= cursegl - (ps * segl);
		}
		/* Force one WordPoint into the very first segment */
		if (i == 2 && ps == 0) {
			ps = 1;
		}
		segment_buf[(i / 2) - 1] = ps;
		x1 = x2;
		y1 = y2;
    }
	return restsegl;
}



/*
 *	Get corner WordPoints sequence from signal. The element of the output sequence is WordPoint and each pair of
 *	WordPoints on the adjacent do not locate on a same key. That means if two WordPoints are both corners and 
 *	adjacent and locate on a same key, one WordPoint will be removed.
 *
 *	Parameters:
 *		filter:			[in] A WordPointer to the global filter.
 *		signal:			[in] A WordPointer to input signal.
 *		n:				[in] The number of WordPoints in signal.
 *		corner:			[out] A WordPointer to output array which saves corner WordPoints. The array is allocated by caller.
 *		m:				[out] Indicate caller the number of WordPoints of output corner sequence.
 */
static void get_corner_from_signal(Filter *aFilter, int16 *signal, int32 n, int16 *corner, int32 *m)
{
	static uint8 cor_key[MAX_CHARS_PER_STRING];
	uint8 *temp;
	uint16 j = 0;
	uint16 i;
	uint8 index;
	EncodingTable *encoding_table = aFilter->encoding_table;
	IndexIgnoreList *index_ignore_list = aFilter->index_ignore_list;

	temp = (uint8 *)q_malloc( n * sizeof(uint8));
	find_corner(signal, n, temp);

	corner[0] = signal[0];
	corner[1] = signal[1];
	cor_key[0] = find_encoded_key(encoding_table, index_ignore_list, signal[0], signal[1]);
	for(i = 1; i < n; i++){
		if(temp[i] == 1){
			index = find_encoded_key(encoding_table, index_ignore_list, signal[2 * i], signal[2 * i + 1]);
			if(cor_key[j] != index){
				j++;
				cor_key[j] = index;
				corner[2 * j] = signal[2 * i];
				corner[2 * j + 1] = signal[2 * i + 1];
			}
		}
	}

	*m = j + 1;
	q_free(temp);
}

/*
 *	Calcuate which WordPoints in signal are corners. If the WordPoint is a corner, set 1; or set 0. The start and end WordPoints
 *	are set 1. For example, input siganl is "x1, x2, x3, x4"(xi is a WordPoint), and x2 is a corner, then the output is
 *	"1, 1, 0, 1".
 *
 *	Parameters:
 *		signal:			[in] A WordPointer to input signal.
 *		n:				[in] The number of WordPoints in signal.
 *		corner:			[out] A WordPointer to uint8 array that indicates which WordPoints are corners. 
 *							  The array is allocated by caller whose size should be n.
 */
static void find_corner(int16 *signal, int32 n, uint8 *corner)
{
	WordPoint *vect, *next, *current;
	int16 *curX, *curY, *nextX, *nextY;
	int32 i = 0;
	double length;
	
	vect = (WordPoint *)q_malloc( n * sizeof(WordPoint));
	n--;
	curX = signal;
	curY = signal + 1;
	nextX = signal + 2;
	nextY = signal + 3;
	while(i < n){
		vect[i].x = *nextX - *curX;
		vect[i].y = *nextY - *curY;
		nextX += 2;
		nextY += 2;
		curX += 2;
		curY += 2;
		i++;
	}

	current = vect;
	next = current + 1;
	i = 1;

	while(i < n){
		length = sqrt(current->x * current->x + current->y * current->y) * sqrt(next->x * next->x + next->y * next->y);
		if(length == 0.0){
			corner[i] = 0;
			i++;
			next++;
			continue;
		}
		if(((current->x * next->x + current->y * next->y) / length) < CORNER_ANGLE_THRESHOLD){
			corner[i] = 1;
		}
		else{
			corner[i] = 0;
		}
		current = next;
		next++;
		i++;
	}
	corner[0] = 1;
	corner[n] = 1;
	q_free(vect);
}

/*
 *	Return edit distance between tow sequences. This is a Dynamic Programming algorithm.
 *
 *	Parameters:
 *		in:				[in] A WordPointer to first WordPoint sequence.
 *		n:				[in] The number of WordPoints of first sequence. (n < MAX_CHARS_PER_STRING)
 *		std:			[in] A WordPointer to second WordPoint sequence. This sequence is often a standard model WordPoint sequence.
 *		m:				[in] The number of WordPoints of second sequence. (m < MAX_CHARS_PER_STRING)
 *		key_width:		[in] The width of key.
 *		key_height:		[in] The height of Key.
 *	
 *	The return value is the only output of the function. In algorithm, we limit the matching path near
 *	by the matrix's diagonal line, that can reduce the time complexity to O(m+n). (We suppose: exist a value N
 *	that makes sure N > abs(m - n) for all m and n.)
 */
static int16 edit_distance(int16 *in, int32 n, int16 *std, int32 m, int16 key_width, int16 key_height)
{
	static int16 mat[MAX_CHARS_PER_STRING][MAX_CHARS_PER_STRING];
	int32 i, j, l, up;
	int16 d1, d2, d3, cur;

	/* Limit matching path */
	memset(mat, 0x11, MAX_CHARS_PER_STRING * MAX_CHARS_PER_STRING * sizeof(int16));
	l = max(2, abs(m - n)) + 1;

	/* Initialize the boundary value */
	mat[0][0] = 3 * compare_distance(in[0], in[1], std[0], std[1], key_width, key_height);
	up = min(n, l);
	for(i = 1; i < up; i++){
		mat[i][0] = mat[i - 1][0] + compare_distance(in[2 * i], in[2 * i +1], std[0], std[1], key_width, key_height);
	}
	up = min(m, l);
	for(j = 1; j < up; j++){
		mat[0][j] = mat[0][j - 1] + compare_distance(in[0], in[1], std[2 * j], std[2 * j + 1], key_width, key_height);
	}

	/* Dynamic Programming algorithm, fill in mat[][] */
	for(i = 1; i < n; i++){
		up = min(m, i + l);
		for(j = max(1, i - l); j < up; j++){
			cur = compare_distance(in[2 * i], in[2 * i + 1], std[2 * j], std[2 * j + 1], key_width, key_height);
			d1 = mat[i][j - 1] + cur;
			d2 = mat[i - 1][j - 1] + cur;
			d3 = mat[i - 1][j] + cur;
			mat[i][j] = min(min(d1, d2), d3);
		}
	}

	/* In order to enhance the last WordPoint weight, re-calculate mat[n-1][m-1], namely the return value */
	cur = 2 * compare_distance(in[2 * n - 2], in[2 * n - 1], std[2 * m - 2], std[2 * m - 1], key_width, key_height);
	d1 = mat[n - 1][m - 2] + cur;
	d2 = mat[n - 2][m - 2] + cur;
	d3 = mat[n - 2][m - 1] + cur;
	return min(min(d1, d2), d3);
}

/*
 *	NOTICE: In order to speed up, this function is defined as a macro.
 *
 *	Return the compare distance between two WordPoints. The distance's min value is 0, and max value is 3.
 *	This distance is essentially 1-norm. Suppose one WordPoint is in the center of a key, if the other WordPoint is also in this
 *	key, the return value is 0. 
 *
 *	Parameters:
 *		x1:				[in] X-coordinate of firest WordPoint.
 *		y1:				[in] Y-coordinate of firest WordPoint.
 *		x2:				[in] X-coordinate of second WordPoint.
 *		y2:				[in] Y-coordinate of second WordPoint.
 *		key_width:		[in] The width of key.
 *		key_heigth:		[in] The height of key.
 *	
 *	The return value is the only output of the funtcion. 
 */
/*
static int16 compare_distance(int16 x1, int16 y1, int16 x2, int16 y2, int16 key_width, int16 key_height)
{
	return min(abs(x1 - x2) / (key_width / 2) + abs(y1 - y2) / (key_height / 2), 3);
}
*/

/*
 *	Return the start direction of a signal. The start direction is defined as a vector. The start WordPoint of the vector is
 *	first WordPoint of the signal, and the end WordPoint is a WordPoint in signal which is just far enough from the start WordPoint. 
 *	The far threshold is provided by caller and is named radius in this function. 
 *
 *	Parameters:
 *		signal:			[in] A WordPointer to input signal. 
 *		n:				[in] The number of WordPoints in signal. 
 *		radius:			[in] The threshold that is used to determine which WordPoint is the end WordPoint of direction vector.
 *							 For caller, it is generally equal to the radius of key's inscribed circle, namely
 *							 radius = min(key_width, key_height) / 2
 *	
 *	The return value is the only output of the funtcion. If can't find a vector which meets the conditions. The return value
 *	is (0, 0).
 */
static WordPoint start_direction(int16 *signal, int32 n, int16 radius)
{
	WordPoint start = {0, 0};
	int32 i;

	if(n < 2){
		return start;
	}
	start.x = signal[0];
	start.y = signal[1];
	for(i = 1; i < n; i++){
		if(distance_noapprox(start.x, start.y, signal[2 * i], signal[2 * i + 1]) >= radius){
			start.x = signal[2 * i] - start.x;
			start.y = signal[2 * i + 1] - start.y;
			return start;
		}
	}
	start.x = 0;
	start.y = 0;
	return start;
}

/* Heap ADT */
static Heap * heap_create(int32 capacity)
{
	Heap *heap;

	heap = q_malloc( sizeof(Heap));
	heap->nodes = q_malloc( (capacity + 1) * sizeof(HeapNode));
	heap->n = 0;
	heap->capacity = capacity;
	return heap;
}

static void heap_delete(Heap *heap)
{
	q_free(heap->nodes);
	q_free(heap);
}

static void heap_clear(Heap *heap)
{
	heap->n = 0;
	memset(heap->nodes, 0, (heap->capacity + 1) * sizeof(HeapNode));
}

static void heap_insert(Heap *heap, float32 distance, uint8 *ptr)
{
	if (heap->n == heap->capacity) {
		if (heap->nodes[1].distance > distance) {
			heap_remove(heap);
		}
		else {
			return;
		}
	}
	heap->n++;
	heap->nodes[heap->n].distance = distance;
	heap->nodes[heap->n].ptr = ptr;
	heap_sift_up(heap, heap->n);
}

static void heap_sift_up(Heap *heap, int32 i)
{
	int32 j;
	HeapNode tmp_node;

	while (i > 1) {
		j = i / 2;
		if (heap->nodes[i].distance > heap->nodes[j].distance) {
			tmp_node = heap->nodes[i];
			heap->nodes[i] = heap->nodes[j];
			heap->nodes[j] = tmp_node;
		}
		else {
			break;
		}
		i = j;
	}
}

static void heap_remove(Heap *heap)
{
	heap->nodes[1] = heap->nodes[heap->n];
	heap->n--;
	heap_sift_down(heap, 1);
}

static void heap_sift_down(Heap *heap, int32 i)
{
	int32 j;
	HeapNode tmp_node;

	while ((j = i * 2) <= heap->n && j > 0) {
		if (j < heap->n && heap->nodes[j].distance < heap->nodes[j + 1].distance) {
			j++;
		}
		if (heap->nodes[i].distance < heap->nodes[j].distance) {
			tmp_node = heap->nodes[i];
			heap->nodes[i] = heap->nodes[j];
			heap->nodes[j] = tmp_node;
		}
		else {
			break;
		}
		i = j;
	}
}

void destroyFilterTable(FilterTable* aFilterTable)
{
    int i = 0;
    if (aFilterTable == NULL) return;
    if (aFilterTable->iFilterItems != NULL) {
        for (i = 0; i < aFilterTable->iNumOfFilterItems; i++) {
            q_free(aFilterTable->iFilterItems[i].iLabel);
        }
    }
    
    q_free(aFilterTable->iFilterItems);
    q_free(aFilterTable);
}

BOOL isQWERTY(FilterParams aParams)
{
    if (aParams.iFilterType == QWERTY) return TRUE;
    return FALSE;
}

#ifdef FILTER_DEBUG
static void heap_dump(Heap *heap, Filter *aFilter)
{
	int32 i, n;
	uint8 *ptr;
	HeapNode node;
	float32 d;
	EncodingTable *encoding_table;
	char str[MAX_BYTES_PER_LEXICON_LINE + 1];

	n = heap->n;
	encoding_table = aFilter->encoding_table;
	for (i = 1; i < n; i++) {
		node = heap->nodes[i];
		ptr = node.ptr;
		d = node.distance;
		memset(str, 0, MAX_BYTES_PER_LEXICON_LINE + 1);
		decode_string(ptr, str, encoding_table);
		SWI_LOG(heap_dump, ("Position:\t%d\tPattern:\t%s\tDistance:\t%f", i, str, d));
	}
}

static void dump_encoding_table(const EncodingTable *encoding_table, const IndexIgnoreList *index_ignore_list)
{
	uint8 i, n;
	EncodingNode *enode;

	n = encoding_table->n;
	SWI_LOG(dump_encoding_table, ("--- Dump Encoding Table ---"));
	SWI_LOG(dump_encoding_table, ("Number of top-level nodes: %d", encoding_table->n));
	SWI_LOG(dump_encoding_table, ("--- Top-level nodes ---"));
	for (i = 0; i < n; i++) {
		SWI_LOG(dump_encoding_table, ("Index: %d Ignore: %d At x: %d y: %d", i, index_ignore_list->ignore_list[i], encoding_table->x_components[i], encoding_table->y_components[i]));
		enode = &(encoding_table->encoding_nodes[i]);
		while (enode != NULL) {
			SWI_LOG(dump_encoding_table, ("\t%c", (char)enode->ec));
			enode = enode->next;
		}
	}
}

static void dump_storage(const uint8 *stg, int32 pattern_count, const EncodingTable *encoding_table)
{
	int32 i, n;
	const uint8 *stg_ptr;
	int32 sz;
	char str[MAX_BYTES_PER_LEXICON_LINE + 1];

	n = pattern_count;
	stg_ptr = stg;
	/* Iterate over the entire storage area */
	for (i = 0; i < n; i++) {
		sz = *stg_ptr;
		memset(str, 0, MAX_BYTES_PER_LEXICON_LINE + 1);
		decode_string(stg_ptr, str, encoding_table);
		SWI_LOG(heap_dump, ("Stg index: %d str: %s", i, str));
		/* Go to next pattern */
		stg_ptr+= sz + PATTERN_HEADER_SIZE;
	}
}

static void dump_trace(EncodingTable *encoding_table, uint8 *trace, int32 n)
{
	register int32 i;
	int32 index;
	char buf[MAX_N_SAMPLE_WordPointS * 4 + 1];

	memset(buf, 0, MAX_N_SAMPLE_WordPointS * 4 + 1);
	for (i = 0; i < n; i++) {
		index = trace[i];
		swi_set_char_at(buf, i, encoding_table->encoding_nodes[index].ec);
	}
	swi_set_char_at(buf, n, '\0');
	SWI_LOG(dump_trace, ("trace: %s", buf));
}
#endif

#ifdef FILTER_LOGGER
static void log_play(FILE *stream)
{
	char label[100];
	int matched;
	SwiInputSignal *input_signal;

	matched = fscanf(stream, "%s", label);
	if (matched == 1 && strcmp(label, "InputSignal") == 0) {
		input_signal = log_read_input_signal(stream);
		if (input_signal != NULL) {
			/* Play the input signal */
			/* Free it */
			q_free(input_signal->sample_WordPoints);
			q_free(input_signal);
			input_signal = NULL;
		}
	}
}

static InputSignal * log_read_input_signal(FILE *stream)
{
	int matched;
	int32 x, y, t;
	int32 sample_WordPoint_count;
	InputSignal *input_signal;
	int32 index;
	fpos_t stream_pos;

	input_signal = NULL;
	fgetpos(stream, &stream_pos);
	/* Count the sample WordPoints */
	sample_WordPoint_count = 0;
	matched = 3;
	while (matched == 3) {
		matched = fscanf(stream, "%d\t%d\t%d\t", &x, &y, &t);
		if (matched == 3) {
			sample_WordPoint_count++;
		}
	}
	fsetpos(stream, &stream_pos);
	/* Create an input signal */
	input_signal = q_malloc( sizeof(SwiInputSignal));
	input_signal->sample_WordPoints = q_malloc( sizeof(SwiSampleWordPoint) * sample_WordPoint_count);
	input_signal->sample_WordPoint_count = sample_WordPoint_count;
	matched = 3;
	index = 0;
	while (matched == 3) {
		matched = fscanf(stream, "%d\t%d\t%d\t", &x, &y, &t);
		if (matched == 3) {
			input_signal->sample_WordPoints[index].x = (float32)x;
			input_signal->sample_WordPoints[index].y = (float32)y;
			input_signal->sample_WordPoints[index].t = (float32)t;
			index++;
		}
	}
	return input_signal;
}

static FILE * log_open()
{
	FILE *stream;

	stream = fopen("C:\\trace.log", "a+");
	fflush(stream);
	return stream;
}

static void log_close(FILE *stream)
{
	fflush(stream);
	fclose(stream);
}

static void log_input_signal(FILE *stream, const InputSignal *input_signal)
{
	int32 i, n;
	int16 x, y, t;

	n = input_signal->sample_WordPoint_count;
	fprintf(stream, "InputSignal");
	fprintf(stream, "\t");
	for (i = 0; i < n; i++) {
		x = (int16)input_signal->sample_WordPoints[i].x;
		y = (int16)input_signal->sample_WordPoints[i].y;
		t = (int16)input_signal->sample_WordPoints[i].t;
		fprintf(stream, "%d\t%d\t%d\t", x, y, t);
	}
	fprintf(stream, "\n");
	fflush(stream);
}

static void log_results(FILE *stream, const Result *results, int32 n, const EncodingTable *encoding_table)
{
	int32 i;
	float32 d;
	char buf[MAX_BYTES_PER_RESULT];

	fprintf(stream, "ResultSet");
	fprintf(stream, "\t");
	for (i = 0; i < n; i++) {
		d = results[i].value;
		memset(buf, 0, MAX_BYTES_PER_RESULT);
		decode_string(results[i].pat_ptr, buf, encoding_table);
		fprintf(stream, "%s\t%f\t", buf, d);
	}
	fprintf(stream, "\n");
	fflush(stream);
}

static void log_probabilities(FILE *stream, float32 *probabilities, int32 n, const EncodingTable *encoding_table, HeapNode *nodes)
{
	int32 i;
	int32 offset;
	float32 p, l, s;
	uint8 *pat_ptr;
	char buf[MAX_BYTES_PER_RESULT];

	fprintf(stream, "Probabilities");
	fprintf(stream, "\t");
	offset = n / 2;
	for (i = 1; i < offset + 1; i++) {
		pat_ptr = nodes[i].ptr;
		memset(buf, 0, MAX_BYTES_PER_RESULT);
		decode_string(pat_ptr, buf, encoding_table);
		l = probabilities[i - 1];
		s = probabilities[i - 1 + offset];
		p = l * s;
		fprintf(stream, "%s\t%f\t%f\t%f\t", buf, p, l, s);
	}
	fprintf(stream, "\n");
	fflush(stream);
}
#endif
