/*
 * cs338-prog2.c
 * Created by Lance Hammond and Kunle Olukotun for CS315a at Stanford University
 * This file provides the "shell" for the first programming assignment
 * for PA1.  It reads in one or more JPEG files at the start, leaving
 * them in buffers that the students can manipulate, and then can compress
 * one or more output files so that results may be compared with input.
 * Currently, the earlier input(s) are input files, and the later ones
 * are output files.
 */

/******************** CONTROLLABLE PARAMETERS *******************/

/* Define this if you want to set the # of runs from the command line.
 * This will be a numerical argument, the first one after the command.
 * Also, feel free to define NO_FIRST if you want to discard the performance
 * of the first run (most likely).
 */
/* #define USE_NUM_RUNS */
/* #define NO_FIRST */

/* Define this if you want to set the # of processors from the command line.
 * This will be a numerical argument, either right after the command or
 * right after the # of runs.
 */
#define USE_NUM_PROCS

/* The following parameters adjust the tolerance of the timing-correction
 * code (0.1 = 10% tolerance, etc.) and the number of out-of-tolerance
 * runs the system will attempt before giving up.  The test delay parameter
 * should be set to check tolerances for at least a couple of seconds.
 */
#define MAX_TOLERANCE_ERRORS	4
#define TOLERANCE				0.1
#define TEST_DELAY				100000000

/* The number of files can be adjusted with the following parameters: */

/* For PA1, you'll need to set this to 1 for Problems 1 & 2, 2 for P.3 */
#define NUM_INPUTS		1
/* If NUM_INPUTS == 0, use these to control a variable # of inputs: */
/*   Not actually necessary for PA1, but could be helpful */
#define MIN_INPUTS		1
#define MAX_INPUTS		1

/* NUM_OUTPUTS is always fixed.  For PA1, 1 for P.1 & 3, 0 for P.2 */
#define NUM_OUTPUTS		1

/* A quality level of 2-100 can be provided (default = 75, high quality = ~95,
 * low quality = ~25, utter pixellation = 2).  This controls the degree of
 * compression at the output.
 */
#define OUT_QUALITY		75

/************************* END OF PARAMETERS *******************/

#ifdef USE_NUM_RUNS
	#define RUNS_COMMAND	1
#else
	#define RUNS_COMMAND	0
#endif

#ifdef USE_NUM_PROCS
	#define RUNS_PROCS		1
#else
	#define RUNS_PROCS		0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "jpeglib.h"

/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 */

/* The "frame structure" structure contains an image frame (in RGB or grayscale
 * formats) for passing around the CS338 projects.
 */
typedef struct frame_struct
{
  JSAMPLE *image_buffer;	/* Points to large array of R,G,B-order/grayscale data
                             * Access directly with:
                             *   image_buffer[num_components*pixel + component]
                             */
  JSAMPLE **row_pointers;	/* Points to an array of pointers to the beginning
                             * of each row in the image buffer.  Use to access
                             * the image buffer in a row-wise fashion, with:
                             *   row_pointers[row][num_components*pixel + component]
                             */
  int image_height;		/* Number of rows in image */
  int image_width;		/* Number of columns in image */
  int num_components;	/* Number of components (usually RGB=3 or gray=1) */
} frame_struct_t;
typedef frame_struct_t *frame_ptr;

/* These arrays of frame structure pointers point to all of the frames that
 * are read at the beginning of the program or written at the end.  Note
 * that you can simply move input frame pointers to output frame pointers if
 * you do "in-place" transformations (leaving output data right in the input
 * buffers).
 */
frame_ptr input_frames[MAX_INPUTS];	/* Pointers to input frames */
frame_ptr output_frames[NUM_OUTPUTS];	/* Pointers to output frames */
int num_procs;		/* Number of processors, for parallel use */

int combination = 0;

/* Function prototypes */

/* Your code */
void CS338_function();

/* Read/write JPEGs, for program startup & shutdown */
/* YOU SHOULD NOT NEED TO USE THESE AT ALL */
void write_JPEG_file (char * filename, frame_ptr p_info, int quality);
frame_ptr read_JPEG_file (char * filename);

/* Allocate/deallocate frame buffers, USE AS NECESSARY! */
frame_ptr allocate_frame(int height, int width, int num_components);
void destroy_frame(frame_ptr kill_me);


/******************** START OF ASSIGNMENT SECTION *******************/

 /* combination 0: interleaved w/ rows in outer loop
  * 1: interleaved w/ columns in the outer loop
  * 2: blocks w/ rows in outer loop
  * 3: blocks w/ columns in outer loop
	*/
 //#define COMBO_0
 //#define COMBO_1
 #define COMBO_2
 //#define COMBO_3

/* The "thread_args" structure contains the image frame, a target frame, and the
 * blur radius for the image. This structure is used so that all threads have
 * access to these variables.
 */
 struct thread_args {
		 frame_ptr from;
		 frame_ptr to;
		 int blur_radius;
 };
/* Arguments shared among threads */
 struct thread_args* args;

 /* Blurs pixel at the given location
  *
	* params:
	* from Original picture frame
	* to Destination frame holding the blurred immage
	* row Row index of pixel
	* column Column index of pixel
	* component Which component we are looking at (R,G,or B)
	* blur_radius The blur radius
  */
 void blurPixel(frame_ptr from, frame_ptr to, int row, int column, int component,
	 				 int blur_radius) {
 	int numerator = 0;
	/* Within r pixels of the edges of the image we need to go as far back as
	 * possible and go as far forward as possible.
	 * For starting, we can't go back -(blur_radius - 1) pixels, the most we can
	 * do is the edge of the frame: go back -row or -column pixels.
	 * For ending, we can't go blur_radius pixels ahead, so we need to go to the
	 * edge of the frame: go forward the remaining pixels.
	 */
	int row_start = row >= blur_radius ? -(blur_radius - 1) : -row;
	int column_start = column >= blur_radius ? -(blur_radius - 1) : -column;
	int row_finish = row < from-> image_height - blur_radius ? blur_radius : from-> image_height - row;
	int column_finish = column < from-> image_width - blur_radius ? blur_radius : from->image_width - column;
 	for (int i = row_start; i < row_finish; i++) {
 		for (int j = column_start; j < column_finish; j++) {
 			numerator += from->row_pointers[row + i]
														[(from->num_components) * (column + j) + component]
 			* (blur_radius - abs(i)) * (blur_radius - abs(j));
 		}
 	}

 	int denominator = 0;
 	for (int i = row_start; i < row_finish; i++) {
 		for (int j = column_start; j < column_finish; j++) {
 			denominator += (blur_radius - abs(i)) * (blur_radius - abs(j));
 		}
 	}

 	int average = numerator/denominator;
 	to->row_pointers[row][(from->num_components)*column+component] = average;

 }

/* Breaks up the blurring so that each thread is assigned 1/N (N = number of
 * thread) rows.
 *
 * params
 * r The rank of the thread
 */
void* chunked_blur_rows(void* r) {

	int chunks = args->from->image_height / num_procs;
	long rank = (long)r;
	int index_from = rank * chunks;
	int index_to = (rank + 1) * chunks;
	int i, j, k;

	for (i=index_from; i < index_to; i++)
		for (j=0; j < args->from->image_width; j++)
			for (k=0; k < args->from->num_components; k++)
				blurPixel(args->from, args->to, i, j, k, args->blur_radius);

	return NULL;
}

/* Breaks up the blurring so that each thread is assigned 1/N (N = number of
 * thread) columns.
 *
 * params
 * r The rank of the thread
 */
void* chunked_blur_columns(void* r) {

	int chunks = args->from->image_width / num_procs;
	long rank = (long)r;
	int index_from = rank * chunks;
	int index_to = (rank + 1) * chunks;
	int i, j, k;
	for (j=index_from; j < index_to; j++)
		for (i=0; i < args->from->image_height; i++)
			for (k=0; k < args->from->num_components; k++)
				blurPixel(args->from, args->to, i, j, k, args->blur_radius);

	return NULL;
}

/* Breaks up the blurring so that it is “blocking” the parallel outer loop on an
 * interleaved basis divided one row at a time.
 *
 * params
 * r The rank of the thread
 */
void* interleaved_blur_row(void* r) {

	long rank = (long)r;
	int i, j, k;
	for (i=0; i < args->from->image_height / num_procs; i++)
		for (j=0; j < args->from->image_width; j++)
			for (k=0; k < args->from->num_components; k++) {
				blurPixel(args->from, args->to, i*num_procs+rank, j, k, args->blur_radius);
			}

	return NULL;
}

/* Breaks up the blurring so that it is “blocking” the parallel outer loop on an
 * interleaved basis divided one column at a time.
 *
 * params
 * r The rank of the thread
 */
void* interleaved_blur_column(void* r) {

	long rank = (long)r;
	int i, j, k;
	for (j=0; j < args->from->image_width / num_procs; j++)
		for (i=0; i < args->from->image_height; i++)
			for (k=0; k < args->from->num_components; k++) {
				blurPixel(args->from, args->to, i, j*num_procs+rank, k, args->blur_radius);
			}

	return NULL;
}

/* Blurs an image */
void CS338_function()
{
	frame_ptr from, to;

	from = input_frames[0];
	to = output_frames[0] = allocate_frame(from->image_height,
		from->image_width, from->num_components);

	int max_dimension = from-> image_height > from->image_width ?
	 											from-> image_height : from->image_width;
	float percent_blur = 0.05;
	int blur_radius = percent_blur * max_dimension;

	long pthread;
	pthread_t ids[num_procs];

	// Store this in a struct so that all threads can read it
	args = malloc (sizeof (struct thread_args));
	args->from = from;
	args->to = to;
	args->blur_radius = blur_radius;

	#ifdef COMBO_0
		for(long i = 0; i < num_procs; i++){
			pthread_create(&ids[i], NULL, interleaved_blur_column, (void*)i);
		}
	#endif
	#ifdef COMBO_1
		for(long i = 0; i < num_procs; i++){
			pthread_create(&ids[i], NULL, interleaved_blur_row, (void*)i);
		}
	#endif
	#ifdef COMBO_2
		for(long i = 0; i < num_procs; i++){
			pthread_create(&ids[i], NULL, chunked_blur_rows, (void*)i);
		}
	#endif
	#ifdef COMBO_3
		for(long i = 0; i < num_procs; i++){
			pthread_create(&ids[i], NULL, chunked_blur_columns, (void*)i);
		}
	#endif

	for(int i = 0; i < num_procs; i++){
		pthread_join(ids[i], NULL);
	}

}

/******************** END OF ASSIGNMENT SECTION *******************/


/*
 * write_JPEG_file writes out the contents of an image buffer to a JPEG.
 * A quality level of 2-100 can be provided (default = 75, high quality = ~95,
 * low quality = ~25, utter pixellation = 2).  Note that unlike read_JPEG_file,
 * it does not do any memory allocation on the buffer passed to it.
 */

void write_JPEG_file (char * filename, frame_ptr p_info, int quality)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */

  /* Step 1: allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "ERROR: Can't open output file %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* Set basic picture parameters (not optional) */
  cinfo.image_width = p_info->image_width; 	/* image width and height, in pixels */
  cinfo.image_height = p_info->image_height;
  cinfo.input_components = p_info->num_components; /* # of color components per pixel */
  if (p_info->num_components == 3)
    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  else if (p_info->num_components == 1)
    cinfo.in_color_space = JCS_GRAYSCALE;
  else {
    fprintf(stderr, "ERROR: Non-standard colorspace for compressing!\n");
    exit(1);
  }
  /* Fill in the defaults for everything else, then override quality */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */
  while (cinfo.next_scanline < cinfo.image_height) {
    (void) jpeg_write_scanlines(&cinfo, &(p_info->row_pointers[cinfo.next_scanline]), 1);
  }

  /* Step 6: Finish compression & close output */

  jpeg_finish_compress(&cinfo);
  fclose(outfile);

  /* Step 7: release JPEG compression object */
  jpeg_destroy_compress(&cinfo);
}


/*
 * read_JPEG_file reads the contents of a JPEG into an image buffer, which
 * is automatically allocated after the size of the image is determined.
 * We want to return a frame struct on success, NULL on error.
 */

frame_ptr read_JPEG_file (char * filename)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile;		/* source file */
  frame_ptr p_info;		/* Output frame information */

  JSAMPLE *realBuffer;
  JSAMPLE **buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  /* Step 1: allocate and initialize JPEG decompression object */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  /* Step 2: open & specify data source (eg, a file) */
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "ERROR: Can't open input file %s\n", filename);
    exit(1);
  }
  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* Step 4: use default parameters for decompression */

  /* Step 5: Start decompressor */
  (void) jpeg_start_decompress(&cinfo);

  /* Step X: Create a frame struct & buffers and fill in the blanks */
  fprintf(stderr, "  Opened %s: height = %d, width = %d, c = %d\n",
      filename, cinfo.output_height, cinfo.output_width, cinfo.output_components);
  p_info = allocate_frame(cinfo.output_height, cinfo.output_width, cinfo.output_components);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, &(p_info->row_pointers[cinfo.output_scanline]), 1);
  }

  /* Step 7: Finish decompression */
  (void) jpeg_finish_decompress(&cinfo);

  /* Step 8: Release JPEG decompression object & file */
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return p_info;
}

/*
 * allocate/destroy_frame allocate a frame_struct_t and fill in the
 *  blanks appropriately (including allocating the actual frames), and
 *  then destroy them afterwards.
 */

frame_ptr allocate_frame(int height, int width, int num_components)
{
  int row_stride;		/* physical row width in output buffer */
  int i;
  frame_ptr p_info;		/* Output frame information */

  /* JSAMPLEs per row in output buffer */
  row_stride = width * num_components;

  /* Basic struct and information */
  if ((p_info = malloc(sizeof(frame_struct_t))) == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failure\n");
    exit(1);
  }
  p_info->image_height = height;
  p_info->image_width = width;
  p_info->num_components = num_components;

  /* Image array and pointers to rows */
  if ((p_info->row_pointers = malloc(sizeof(JSAMPLE *) * height)) == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failure\n");
    exit(1);
  }
  if ((p_info->image_buffer = malloc(sizeof(JSAMPLE) * row_stride * height)) == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failure\n");
    exit(1);
  }
  for (i=0; i < height; i++)
  	p_info->row_pointers[i] = & (p_info->image_buffer[i * row_stride]);

  /* And send it back! */
  return p_info;
}

void destroy_frame(frame_ptr kill_me)
{
	free(kill_me->image_buffer);
	free(kill_me->row_pointers);
	free(kill_me);
}

/* This routine sets the CPU-time vs. real-time correction factor */
double get_correction(int initial)
{
	clock_t start_clock, end_clock;	/* Clock for getting time "reference" */
	struct timeval start_time, end_time;	/* Time before/after "reference" */
	static double cpu_time;
	double real_time, correction;
	volatile int i, sum;

	/* Record start times */
	start_clock = clock();
	gettimeofday(&start_time, NULL);

	/* Delay awhile while holding the CPU */
	for (i=0; i < TEST_DELAY; i++)
		sum += 1;

	/* Get end times */
	end_clock = clock();
	gettimeofday(&end_time, NULL);

	/* Calculate the correction factor */
	if (initial)	/* Assume that CPU time doesn't change for later calls */
		cpu_time = ((double) (end_clock-start_clock)) / CLOCKS_PER_SEC;
	real_time = ((double)(end_time.tv_sec) + (double)(end_time.tv_usec)/1000000.0)
		- ((double)(start_time.tv_sec) + (double)(start_time.tv_usec)/1000000.0);
	correction = cpu_time / real_time;
	fprintf(stderr, "testing: CPU %10.3f vs. Real %10.3f: correction factor = %10.6f\n",
		cpu_time, real_time, correction);
	return correction;
}

/* The shell's main routine.  Consists of several steps:
 *	1> Reads JPEGs using first command-line parameter(s)
 *  2> Records the process's elapsed time
 *  3> Lets the user play with the images
 *  4> Checks the elapsed time again, prints out the difference
 *  5> Writes JPEGs out using last command-line parameter(s)
 */
int main(int argc, char *argv[])
{
	int i;
	int num_inputs;	/* Number of JPEG input-output files */
	int num_runs, run, discards; /* Number of runs, for multi-run code */
	clock_t start_clock, end_clock;	/* Clock for getting time "reference" */
	double run_time, total_time; /* Amount of time we've run */
	double correction, new_correction; /* CPU-vs.-real time correction */
	struct timeval start_time, end_time;	/* Time before/after user code */

	/* Step 0A: Check & process command line */
	if (NUM_INPUTS > 0)
	{
		if (argc != 1 + NUM_INPUTS + NUM_OUTPUTS + RUNS_COMMAND + RUNS_PROCS) {
			fprintf(stderr, "ERROR: Wrong number of inputs\n");
			exit(1);
		}
	}
	else
	{
		if (argc < 1 + MIN_INPUTS + NUM_OUTPUTS + RUNS_COMMAND + RUNS_PROCS) {
			fprintf(stderr, "ERROR: Too few input filenames\n");
			exit(1);
		}
		if (argc > 1 + MAX_INPUTS + NUM_OUTPUTS + RUNS_COMMAND + RUNS_PROCS) {
			fprintf(stderr, "ERROR: Too many input filenames\n");
			exit(1);
		}
	}
	num_inputs = argc - 1 - NUM_OUTPUTS - RUNS_COMMAND - RUNS_PROCS;

	#ifdef USE_NUM_RUNS
		num_runs = atoi(argv[1]);
		if (num_runs < 1) num_runs = 1;
		if (num_runs > 10) num_runs = 10; /* Change if you like LOTS of runs */
		fprintf(stderr, "Making %d runs . . .\n", num_runs);
	#endif
	#ifdef USE_NUM_PROCS
		num_procs = atoi(argv[1 + RUNS_COMMAND]);
		if (num_procs < 1) num_procs = 1;
		if (num_procs > 16) num_procs = 16;
		fprintf(stderr, "Using %d processors . . .\n", num_procs);
	#endif

	/* Step 1: Get some JPEGs into memory, uncompressed */
	for (i=0; i < num_inputs; i++)
		input_frames[i] = read_JPEG_file(argv[i+1+RUNS_COMMAND+RUNS_PROCS]);

	/* Loop over multiple runs, if desired */
	total_time = 0.0;
	correction = get_correction(1);	/* Extra one at start to warm up */
	correction = get_correction(1);
	run = discards = 0;
	#ifdef USE_NUM_RUNS
	while ((run < num_runs) && (discards < MAX_TOLERANCE_ERRORS))
	#endif
	{
		/* Step 2: Record elapsed time */
		start_clock = clock();
		gettimeofday(&start_time, NULL);

		/* Step 3: Call a user function! */
		CS338_function();

		/* Step 4: Check & print elapsed time */
		gettimeofday(&end_time, NULL);
		end_clock = clock();
		run_time = ((double)(end_time.tv_sec) + (double)(end_time.tv_usec)/1000000.0)
			- ((double)(start_time.tv_sec) + (double)(start_time.tv_usec)/1000000.0);

		/* Measure correction factor (parallel threads must be blocked) */
		new_correction = get_correction(0);
		if ((new_correction > (1.0-TOLERANCE)*correction) &&
			(new_correction < (1.0+TOLERANCE)*correction))
		{
			fprintf(stderr, "%d. ELAPSED  TIME = %20.3f sec, corrected to %20.3f sec\n",
				run, run_time, run_time*correction);
			total_time += run_time*correction;
			#ifdef NO_FIRST
				if (run == 0)
				{
					fprintf(stderr, "  . . . first run discarded\n");
					total_time = 0.0;
				}
			#endif
			run++;
		}
		else
		{
			fprintf(stderr, "X. ELAPSED    TIME = %20.3f sec, discarding: correction varying too much\n",
				run_time);
			discards++;
		}
		fprintf(stderr, "   TOTAL CPU TIME = %20.3f sec\n",
			((double) (end_clock-start_clock)) / CLOCKS_PER_SEC);
		correction = new_correction;
	}

	/* Print out final, average time, if desired */
	#ifdef USE_NUM_RUNS
		#ifdef NO_FIRST
			fprintf(stderr, "AVERAGE CORRECTED ELAPSED TIME = %20.3f seconds\n",
				total_time / ((double)(run - 1)));
		#else
			fprintf(stderr, "AVERAGE CORRECTED ELAPSED TIME = %20.3f seconds\n",
				total_time / ((double)run));
		#endif
	#endif

	/* Step 5: Write JPEGs out from memory buffers */
	for (i=0; i < NUM_OUTPUTS; i++)
		write_JPEG_file(argv[argc - NUM_OUTPUTS + i], output_frames[i], OUT_QUALITY);
}
