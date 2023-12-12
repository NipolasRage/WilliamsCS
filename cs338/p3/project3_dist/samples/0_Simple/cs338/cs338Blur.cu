/**
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */


#include <stdio.h>
#include <math.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

#include <helper_cuda.h>



////////////////////////////////////////////////////////////////////////////////


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




#define MAXINPUTS 1
#define MAXOUTPUTS 1
frame_ptr input_frames[MAXINPUTS];	/* Pointers to input frames */
frame_ptr output_frames[MAXOUTPUTS];	/* Pointers to output frames */

/* Read/write JPEGs, for program startup & shutdown */
/* YOU SHOULD NOT NEED TO USE THESE AT ALL */
void write_JPEG_file (char * filename, frame_ptr p_info, int quality);
frame_ptr read_JPEG_file (char * filename);

/* Allocate/deallocate frame buffers, USE AS NECESSARY! */
frame_ptr allocate_frame(int height, int width, int num_components);
void destroy_frame(frame_ptr kill_me);

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

  //  JSAMPLE *realBuffer;
  //  JSAMPLE **buffer;		/* Output row buffer */
  //  int row_stride;		/* physical row width in output buffer */

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
  if ((p_info = (frame_struct_t*)malloc(sizeof(frame_struct_t))) == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failure\n");
    exit(1);
  }
  p_info->image_height = height;
  p_info->image_width = width;
  p_info->num_components = num_components;

  /* Image array and pointers to rows */
  if ((p_info->row_pointers = (JSAMPLE**)malloc(sizeof(JSAMPLE *) * height)) == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failure\n");
    exit(1);
  }
  if ((p_info->image_buffer = (JSAMPLE*)malloc(sizeof(JSAMPLE) * row_stride * height)) == NULL) {
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


void usage()
{
  fprintf(stderr, "ERROR: Need to specify input file and then output file\n");
  exit(1);
}


/* Makes sure values match in the two images*/
void checkResults(frame_ptr f1, frame_ptr f2)
{
  int i, j, k;

  if(f1->image_height != f2->image_height && f1->image_width != f2->image_width
		&& f1->num_components != f2->num_components){
	fprintf(stderr, "Dimensions do not match\n");
	exit(1);
  }

  for (i=0; i < f1->image_height; i++){
    for (j=0; j < f1->image_width; j++){
      for (k=0; k < f1->num_components; k++){
		JSAMPLE j1 = f1->row_pointers[i][(f1->num_components)*j+k];
		JSAMPLE j2 = f2->row_pointers[i][(f2->num_components)*j+k];
		if(j1 != j2){
			fprintf(stderr, "Values do not match at (%d, %d, %d) \n", i, j, k);
			fprintf(stderr, "from %d\n", j1);
			fprintf(stderr, "to %d\n", j2);
			exit(1);
		}
      }
    }
  }

}

void runKernel(frame_ptr input, frame_ptr result);

void uni_blur(frame_ptr from, frame_ptr to) {
  int width = from->image_width;
  int height = from->image_height;

  int max_dimension = height > width ?
                        height : width;
  float percent_blur = 0.05;
  int blur_radius = percent_blur * max_dimension;

  for (int row = 0; row < height; row++)
    for (int column = 0; column < width; column++)
      for (int k = 0; k < from->num_components; k++) {
        int numerator = 0;
        int denominator = 0;
        for (int i = -(blur_radius - 1); i < blur_radius; i++) {
          // if out of bounds
          if (i + column < 0 || i + column >= width)
            continue;

            for (int j = -(blur_radius - 1); j < blur_radius; j++) {
              // if out of bounds
              if(j + row < 0 || j + row >= height)
                   continue;
              numerator += from->row_pointers[row + j][(from->num_components) * (column + i) + k] *
                           (blur_radius - abs(i)) * (blur_radius - abs(j));

              denominator += (blur_radius - abs(i)) * (blur_radius - abs(j));
            }

        }
        int average = numerator/denominator;
        to->row_pointers[row][(from->num_components)*column+k] = average;
      }
}


/*
 * This is just a helper method. It should call runKernel to set up and
 * invoke the kernel.  It should then also call the uniprocessor version
 * of your blurring code (which does not need to be optimized) and
 * check for correctness of your kernel code.
 */
void
runTest( int argc, char** argv)
{

  frame_ptr from = input_frames[0];
  // Allocate frame for kernel to store its results into
  output_frames[0] = allocate_frame(from->image_height, from->image_width, from->num_components);

  frame_ptr to = output_frames[0];

  // initialize output array to all black
  for (int i=0; i < from->image_height; i++)
    for (int j = 0; j < from->image_width * from->num_components; j++)
      to->row_pointers[i][j] = 0;

  // call kernel
  runKernel(from, to);

  // invoke uniprocessor version and check results of kernel to uniprocessor
  // version
  // frame_ptr uni_to = allocate_frame(from->image_height, from->image_width, from->num_components);
  // uni_blur(from, uni_to);
  // checkResults(to, uni_to);

}

/* Turns i and j displacement into single dimension index for the flat array
 * of weights.
 *
 * i and j must be within [-(r-1), r-1]
 *
 * params
 * r blur radius
 * i,j i displacement
 */
__device__ int index_to_weights(int r, int i, int j) {
  int row = i - 1 + r;
  int column = j - 1 + r;
  return row * ((2 * (r-1)) + 1) + column;
}

/* Returns an array of precomputed the weights
 *
 * size must represent the cardinality of [-(r - 1), -1] U [0] U [1, r-1]
 *
 * params
 * r Blur radius
 * size Size of weight array
 */
int* precompute_weights(int r, int size) {
  int* weights = (int*) malloc( size * sizeof(int) );

  int counter = 0;
  for (int i = -(r-1); i < r; i++)
    for (int j = -(r-1); j < r; j++)
      weights[counter++] =  (r - abs(i)) * (r - abs(j));

  return weights;

}

/* Returns division factor
 *
 * params
 * r Blur radius
 */
int compute_division_factor(int r) {
  int division_factor = 0;
  for (int i = -(r-1); i < r; i++)
    for (int j = -(r-1); j < r; j++)
      division_factor += (r - abs(i)) * (r - abs(j));

  return division_factor;
}

/**
 * CUDA Kernel Device code
 * This is code for blurring a single pixel
 *
*/

// naive kernel without branching
__global__ void cs338Blur(unsigned char* from, unsigned char* to,
                          int blur_radius, int height, int width,
                          int num_components, int n)
{
  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index < n) {

    // computes values related to 2D frame
    int thread_x = (index / num_components) % width;
    int thread_y = (index / num_components) / width;
    int thread_k = index % num_components;

    int numerator = 0;
    int denominator = 0;
    for (int i = -(blur_radius - 1); i < blur_radius; i++) {
      // if out of bounds
      if(i + thread_y < 0 || i + thread_y >= height)
           continue;

        for (int j = -(blur_radius - 1); j < blur_radius; j++) {
          // if out of bounds
          if (j + thread_x < 0 || j + thread_x >= width)
            continue;

          numerator += from[((thread_y + i) * width * num_components + (thread_x + j) * num_components) + thread_k] *
                       (blur_radius - abs(i)) * (blur_radius - abs(j));

          denominator += (blur_radius - abs(i)) * (blur_radius - abs(j));
        }

    }

    to[index] = numerator/denominator;
  }

}

// kernel that reduces branching
__global__ void cs338Blur2(unsigned char* from, unsigned char* to,
                          int blur_radius, int height, int width,
                          int num_components, int n)
{
  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index < n) {

    // computes values related to 2D frame
    int thread_x = (index / num_components) % width;
    int thread_y = (index / num_components) / width;
    int thread_k = index % num_components;

    /* (Thinking in 2D)
     * Within r pixels of the edges of the image we need to go as far back as
     * possible and go as far forward as possible.
     * For starting, we can't go back -(blur_radius - 1) pixels, the most we can
     * do is the edge of the frame: go back -thread_y or -thread_x pixels.
     * For ending, we can't go blur_radius pixels ahead, so we need to go to the
     * edge of the frame: go forward the remaining pixels.
     */
    int row_start = thread_y >= blur_radius ? -(blur_radius - 1) : -thread_y;
    int column_start = thread_x >= blur_radius ? -(blur_radius - 1) : -thread_x;
    int row_finish = thread_y < height - blur_radius ? blur_radius : height - thread_y;
    int column_finish = thread_x < width - blur_radius ? blur_radius : width - thread_x;

    int numerator = 0;
    int denominator = 0;
    for (int i = row_start; i < row_finish; i++) {
        for (int j = column_start; j < column_finish; j++) {

          numerator += from[((thread_y + i) * width * num_components + (thread_x + j) * num_components) + thread_k] *
                       (blur_radius - abs(i)) * (blur_radius - abs(j));

          denominator += (blur_radius - abs(i)) * (blur_radius - abs(j));
        }

    }

    to[index] = numerator/denominator;
  }

}

// kernel that has branches for valid and invalid pixels and where weights and
// division factor are precomputed
__global__ void cs338Blur3(unsigned char* from, unsigned char* to,
                             int blur_radius, int height, int width,
                             int num_components, int n, int* weights,
                             int division_factor)
{
  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index < n) {

    // computes values related to 2D frame
    int thread_x = (index / num_components) % width;
    int thread_y = (index / num_components) / width;
    int thread_k = index % num_components;

    // if valid pixels
    if (thread_x >= blur_radius - 1 && thread_x < width - (blur_radius - 1) &&
        thread_y >= blur_radius - 1 && thread_y < height - (blur_radius - 1)) {
          // then use weights array and division factor
          int numerator = 0;
          for (int i = -(blur_radius - 1); i < blur_radius; i++) {
              for (int j = -(blur_radius - 1); j < blur_radius; j++) {

                numerator += from[((thread_y + i) * width * num_components + (thread_x + j) * num_components) + thread_k] *
                             weights[index_to_weights(blur_radius, i, j)];

              }

          }

          to[index] = numerator/division_factor;
    } // else the pixels are around the edge
    else {

      /* (Thinking in 2D)
       * Within r pixels of the edges of the image we need to go as far back as
       * possible and go as far forward as possible.
       * For starting, we can't go back -(blur_radius - 1) pixels, the most we can
       * do is the edge of the frame: go back -thread_y or -thread_x pixels.
       * For ending, we can't go blur_radius pixels ahead, so we need to go to the
       * edge of the frame: go forward the remaining pixels.
       */
      int row_start = thread_y >= blur_radius ? -(blur_radius - 1) : -thread_y;
      int column_start = thread_x >= blur_radius ? -(blur_radius - 1) : -thread_x;
      int row_finish = thread_y < height - blur_radius ? blur_radius : height - thread_y;
      int column_finish = thread_x < width - blur_radius ? blur_radius : width - thread_x;

      int numerator = 0;
      int denominator = 0;
      // still uses weights array
      for (int i = row_start; i < row_finish; i++) {
          for (int j = column_start; j < column_finish; j++) {

            numerator += from[((thread_y + i) * width * num_components + (thread_x + j) * num_components) + thread_k] *
                         weights[index_to_weights(blur_radius, i, j)];

            denominator += (blur_radius - abs(i)) * (blur_radius - abs(j));

          }

      }

      /* Cannot use division factor because I cannot pass in different factors
       * to account for invalid pixels.
       */
      to[index] = numerator/denominator;
    }
  }

}




/**
 * Host main routine
 */
int
main(int argc, char **argv)
{

  if(argc < 3){
    usage();
    exit(1);
  }

  // Load input file
  input_frames[0] = read_JPEG_file(argv[1]);

  // Do the actual work including calling CUDA kernel
  runTest(argc, argv);

  // Write output file
  write_JPEG_file(argv[2], output_frames[0], 75);

  return 0;
}

//********************************************************************************************************************************************


// This sets up GPU device by allocating the required memory and then
// calls the kernel on GPU. (You might choose to add/remove arguments.)
// It's currently set up to use the global variables and write its
// final results into the specified argument.
void
runKernel(frame_ptr input, frame_ptr result)
{

  // calculates blur radius
  int max_dimension = input-> image_height > input->image_width ?
                        input-> image_height : input->image_width;
  float percent_blur = 0.05;
  int blur_radius = percent_blur * max_dimension;

  // size of flattened array
  int size = input->image_height * input->image_width * input->num_components * sizeof(unsigned char);

  unsigned char* input_d; // ptr to input image
  unsigned char* result_d; // ptr to output image

  // 1. Transfer input to device memory
  checkCudaErrors(cudaMalloc((void **) &input_d, size));
  checkCudaErrors(cudaMemcpy(input_d, input->image_buffer, size, cudaMemcpyHostToDevice));

  // Allocate device memory for result
  checkCudaErrors(cudaMalloc((void **) &result_d, size));

  // 2. Kernel invocation code
  // 32 is the maximum number for this dimension (32, 16, 8, 4, 2)
  int blockDim = 32;
  dim3 DimGrid(sqrt(size/(blockDim * blockDim)), sqrt(size/(blockDim * blockDim)), 1);
  // takes ceiling of x and y dimensions
  if (sqrt(size/(blockDim * blockDim))) {
    DimGrid.x++;
    DimGrid.y++;
  }
  dim3 DimBlock(blockDim, blockDim, 1);

  // for part 3 we need to precompute the weights and the division factor
  #define PART3
  #ifdef PART3
    int* weights;
    int* weights_d; // ptr to weights in device

    // The 2D array is square and has a row of size of [-(r - 1), -1] U [0] U [1, r-1]
    int weight_size = ((2 * (blur_radius-1)) + 1) * ((2 * (blur_radius-1)) + 1) * sizeof(int);
    weights = precompute_weights(blur_radius, weight_size);
    int division_factor = compute_division_factor(blur_radius);

    checkCudaErrors(cudaMalloc((void **) &weights_d, weight_size));
    checkCudaErrors(cudaMemcpy(weights_d, weights, weight_size, cudaMemcpyHostToDevice));

  #endif

  // Uses cudaEvent_t to get timing information
  cudaEvent_t start, stop;
  float time;

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);

  // cs338Blur<<<DimGrid, DimBlock>>>(input_d, result_d, blur_radius,
  //                                  input-> image_height, input-> image_width,
  //                                  input->num_components, size);
  // cs338Blur2<<<DimGrid, DimBlock>>>(input_d, result_d, blur_radius,
  //                                  input-> image_height, input-> image_width,
  //                                  input->num_components, size);
  #ifdef PART3
  cs338Blur3<<<DimGrid, DimBlock>>>(input_d, result_d, blur_radius,
                                      input-> image_height, input-> image_width,
                                      input->num_components, size, weights_d,
                                      division_factor);
  #endif

  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);

  printf("Time to blur image:  %3.1f ms \n", time);

  // 3. Transfer result from device to host
  checkCudaErrors(cudaMemcpy(result->image_buffer, result_d, size, cudaMemcpyDeviceToHost));

  // Free device memory for input, result
  cudaFree(input_d);
  cudaFree(result_d);
  #ifdef PART3
    cudaFree(weights_d);
  #endif
}
