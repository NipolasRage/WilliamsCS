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

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

#include <helper_cuda.h>
#include <time.h>


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

void runKernel(frame_ptr result);

// input->num_components should be 1 as the input got changed to grayscale
frame_ptr pad_frame(frame_ptr input) {
  int new_height = input->image_height+2;
  int new_width = input->image_width+2;
  frame_ptr padded = allocate_frame(new_height, new_width, input->num_components);

  // write old values into our new padded image
  for (int row = 0; row < input->image_height; row++)
    for (int col = 0; col < input->image_width; col++)
      padded->row_pointers[row+1][col+1] = input->row_pointers[row][col];

  // extends pixels to first and last columns
  for (int row = 1; row < new_height-1; row++) {
    padded->row_pointers[row][0] = padded->row_pointers[row][1];
    padded->row_pointers[row][new_width-1] = padded->row_pointers[row][new_width-2];
  }

  // extends top and bottom rows
  for (int column = 0; column < new_width; column++) {
    padded->row_pointers[0][column] = padded->row_pointers[1][column];
    padded->row_pointers[new_height-1][column] = padded->row_pointers[new_height-2][column];
  }

  return padded;
}

/* Kernel that runs the edge detection algorithm on each output pixel with
 * hard coded sobel filters
 *
 * input has width and height that's two bigger than to's because of the way
 * convolution works
 */
__global__ void register_memory_edge_detection(unsigned char* input, unsigned char* to,
                               int to_size, int to_width, int to_height,
                               int from_width, int from_height) {

  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index >= to_size) return;

  // computes values related to param to's 2D frame
  int thread_x = index % to_width;
  int thread_y = index / to_width;

  // frame in original picture needed for convolution
  int frame[3][3] = {
    {input[(thread_y) * from_width + (thread_x)], input[(thread_y) * from_width + (thread_x + 1)], input[(thread_y) * from_width + (thread_x + 2)]},
    {input[(thread_y + 1) * from_width + (thread_x)], input[(thread_y + 1) * from_width + (thread_x + 1)], input[(thread_y + 1) * from_width + (thread_x + 2)]},
    {input[(thread_y + 2) * from_width + (thread_x)], input[(thread_y + 2) * from_width + (thread_x + 1)], input[(thread_y + 2) * from_width + (thread_x + 2)]}
  };

  // kernels are hard coded
  /* x kernel:
   * [-1, 0 , 1]
   * [-2, 0, 2]
   * [-1, 0, 1]
   */
  int x_convolution = frame[0][2] - frame[0][0]
                      + 2 * frame[1][2] - 2 * frame[1][0]
                      + frame[2][2] - frame[2][0];

  /* y kernel:
   * [-1, -2 , -1]
   * [0, 0, 0]
   * [1, 2, 1]
   */
  int y_convolution = - frame[0][0] - 2 * frame[0][1] - frame[0][2]
                      + frame[2][0] + 2 * frame[2][1] + frame[2][2];

  int gradient_magnitude = sqrt((float)((x_convolution * x_convolution) + (y_convolution * y_convolution)));

  to[index] = gradient_magnitude;
}

/* Kernel that runs the edge detection algorithm on each output pixel with
 * sobel filter as input (on global memory)
 *
 * input has width and height that's two bigger than to's because of the way
 * convolution works
 */
__global__ void global_memory_edge_detection(unsigned char* input, unsigned char* to,
                               int to_size, int to_width, int to_height,
                               int from_width, int from_height,
                               int* x_filter, int* y_filter) {

  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index >= to_size) return;

  // computes values related to param to's 2D frame
  int thread_x = index % to_width;
  int thread_y = index / to_width;

  // frame in original picture needed for convolution
  // size is 3 by 3 because each sobel filter is 3 by 3
  int frame[3][3] = {
    {input[(thread_y) * from_width + (thread_x)], input[(thread_y) * from_width + (thread_x + 1)], input[(thread_y) * from_width + (thread_x + 2)]},
    {input[(thread_y + 1) * from_width + (thread_x)], input[(thread_y + 1) * from_width + (thread_x + 1)], input[(thread_y + 1) * from_width + (thread_x + 2)]},
    {input[(thread_y + 2) * from_width + (thread_x)], input[(thread_y + 2) * from_width + (thread_x + 1)], input[(thread_y + 2) * from_width + (thread_x + 2)]}
  };

  int x_convolution = 0;
  int filter_index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      x_convolution += frame[i][j] * x_filter[filter_index];
      filter_index++;
    }
  }

  int y_convolution = 0;
  filter_index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      y_convolution += frame[i][j] * y_filter[filter_index];
      filter_index++;
    }
  }

  int gradient_magnitude = sqrt((float)((x_convolution * x_convolution) + (y_convolution * y_convolution)));

  to[index] = gradient_magnitude;
}

/* Kernel that runs the edge detection algorithm on each output pixel with
 * sobel filters declared on shared memory.
 *
 * input has width and height that's two bigger than to's because of the way
 * convolution works
 */
__global__ void shared_memory_edge_detection(unsigned char* input, unsigned char* to,
                               int to_size, int to_width, int to_height,
                               int from_width, int from_height) {

  // computes which block this thread lies on
  int blockId = blockIdx.x + (gridDim.x * blockIdx.y);

  // computes the index of the thread
  int index = (blockId * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.y) + threadIdx.x;

  if (index >= to_size) return;

  // computes values related to param to's 2D frame
  int thread_x = index % to_width;
  int thread_y = index / to_width;

  // frame in original picture needed for convolution
  // size is 3 by 3 because each sobel filter is 3 by 3
  int frame[3][3] = {
    {input[(thread_y) * from_width + (thread_x)], input[(thread_y) * from_width + (thread_x + 1)], input[(thread_y) * from_width + (thread_x + 2)]},
    {input[(thread_y + 1) * from_width + (thread_x)], input[(thread_y + 1) * from_width + (thread_x + 1)], input[(thread_y + 1) * from_width + (thread_x + 2)]},
    {input[(thread_y + 2) * from_width + (thread_x)], input[(thread_y + 2) * from_width + (thread_x + 1)], input[(thread_y + 2) * from_width + (thread_x + 2)]}
  };

  // kernels are now in shared memory
  __shared__ int x_filter[9];
  int x_filter_h[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
  for (int i = 0; i < 9; i++)
    x_filter[i] = x_filter_h[i];
  __shared__ int y_filter[9];
  int y_filter_h[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
  for (int i = 0; i < 9; i++)
    y_filter[i] = y_filter_h[i];

  int x_convolution = 0;
  int filter_index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      x_convolution += frame[i][j] * x_filter[filter_index];
      filter_index++;
    }
  }

  int y_convolution = 0;
  filter_index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      y_convolution += frame[i][j] * y_filter[filter_index];
      filter_index++;
    }
  }

  int gradient_magnitude = sqrt((float)((x_convolution * x_convolution) + (y_convolution * y_convolution)));

  to[index] = gradient_magnitude;
}

void serial_edge_detection(frame_ptr input, frame_ptr to) {
  for (int y = 0; y < to -> image_height; y++)
    for (int x = 0; x < to -> image_width; x++) {
      // b22 = a13 - a11 + 2a23 - 2a21 + a33 - a31
      // since the input height and width are two more than to's height and width
      // then we need to offset x and y by 1. For example:
      // b00 = a02 - a00 + 2a13 - 2a10 + a22 - a20
            int x_convolution = input->row_pointers[y][x+2] - input->row_pointers[y][x]
	                              + 2 * input->row_pointers[y+1][x+2] - 2 * input->row_pointers[y+1][x]
	      + input->row_pointers[y+2][x+2] - input->row_pointers[y+2][x];

	    // b22 = a11 - 2a12 - a13 + a31 + 2a32 + a33
	    // similar offsets as above
	          int y_convolution = - input->row_pointers[y][x] - 2 * input->row_pointers[y][x+1] - input->row_pointers[y][x+2]
		    + input->row_pointers[y+2][x] + 2 * input->row_pointers[y+2][x+1] + input->row_pointers[y+2][x+2];

		  int gradient_magnitude = sqrt((x_convolution * x_convolution) + (y_convolution * y_convolution));
		  to -> row_pointers[y][x] = gradient_magnitude;
    }
}

void run_cuda_edge_detection(frame_ptr input, frame_ptr to) {
  // size of flattened array (to_components should be 1)
  // input's height and width should both be two more than to's
  int to_size = to->image_height * to->image_width * sizeof(unsigned char);
  int input_size = input->image_height * input->image_width * sizeof(unsigned char);

  unsigned char* input_d; // ptr to input image
  unsigned char* result_d; // ptr to output image
  int* sobel_x_filter_d; // ptr to sobel x filter
  int* sobel_y_filter_d; // ptr to sobel x filter

  // 1. Transfer input to device memory
  // input image
  checkCudaErrors(cudaMalloc((void **) &input_d, input_size));
  checkCudaErrors(cudaMemcpy(input_d, input->image_buffer, input_size, cudaMemcpyHostToDevice));

  // sobel filter
  int x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
  int y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
  int (*sobel_x_filter)[9] = &x;
  int (*sobel_y_filter)[9] = &y;
  int filter_size = 9 * sizeof(int);
  checkCudaErrors(cudaMalloc((void **) &sobel_x_filter_d, filter_size));
  checkCudaErrors(cudaMemcpy(sobel_x_filter_d, sobel_x_filter, filter_size, cudaMemcpyHostToDevice));

  checkCudaErrors(cudaMalloc((void **) &sobel_y_filter_d, filter_size));
  checkCudaErrors(cudaMemcpy(sobel_y_filter_d, sobel_y_filter, filter_size, cudaMemcpyHostToDevice));

  // Allocate device memory for result
  checkCudaErrors(cudaMalloc((void **) &result_d, to_size));

  // 2. Kernel invocation code
  // 32 is the maximum number for this dimension (32, 16, 8, 4, 2)
  int blockDim = 32;
  dim3 DimGrid(to->image_width / blockDim, to->image_height / blockDim, 1);
  // takes ceiling of x and y dimensions
  if (to->image_width % blockDim != 0) DimGrid.x++;
  if (to->image_height % blockDim != 0) DimGrid.y++;

  dim3 DimBlock(blockDim, blockDim, 1);

  // Uses cudaEvent_t to get timing information
  cudaEvent_t start, stop;
  float time;

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);

  register_memory_edge_detection<<<DimGrid, DimBlock>>>(input_d, result_d, to_size,
                                        to-> image_width, to-> image_height,
                                        input->image_width, input->image_height);

  // global_memory_edge_detection<<<DimGrid, DimBlock>>>(input_d, result_d, to_size,
  //                                        to-> image_width, to-> image_height,
  //                                        input->image_width, input->image_height,
  //                                        sobel_x_filter_d, sobel_y_filter_d);

 // shared_memory_edge_detection<<<DimGrid, DimBlock>>>(input_d, result_d, to_size,
 //                                       to-> image_width, to-> image_height,
 //                                       input->image_width, input->image_height);

  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);

  printf("Time run edge detection:  %3.1f ms \n", time);

  // 3. Transfer result from device to host
  checkCudaErrors(cudaMemcpy(to->image_buffer, result_d, to_size, cudaMemcpyDeviceToHost));

  // Free device memory for input, result
  cudaFree(input_d);
  cudaFree(result_d);
  cudaFree(sobel_x_filter_d);
  cudaFree(sobel_y_filter_d);
}


void rgb2grayscale(frame_ptr in, frame_ptr out) {
  unsigned char pixel[3]; // holds rgb value for a given pixel

  for (int row = 0; row < in->image_height; row++) {
    for (int col = 0; col < in->image_width; col++) {
      for (int k=0; k < in->num_components; k++) {
         pixel[k] = in->row_pointers[row][col*in->num_components+k];
       }
       out->row_pointers[row][col] = pixel[0] * 0.3 + pixel[1] * 0.59 + pixel[2] * 0.11;
     }
   }
}


void printPixels(frame_ptr in, char *filename) {

  FILE *fp;
  fp = fopen(filename, "w+");

  for (int i=0; i < in->image_height; i++){
    for (int j=0; j < in->image_width; j++){
      for (int k=0; k < in->num_components; k++){
		     JSAMPLE j1 = in->row_pointers[i][(in->num_components)*j+k];
          fprintf(fp, "%d", j1);
      }
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}




/*
 * This is just a helper method. It sets the input and output frames
 * and calls edge detection either serially or on cuda
 */
void
runTest( int argc, char** argv)
{

  frame_ptr from = input_frames[0];
  // Allocate frame for kernel to store its results into
  output_frames[0] = allocate_frame(from->image_height, from->image_width, 1);

  frame_ptr testing_frame = allocate_frame(from->image_height, from->image_width, 1);
  //convert image to grayscale
  rgb2grayscale(from, output_frames[0]);

  //printPixels(gray, "gray.txt");

  //pad grayscale image
  frame_ptr img = pad_frame(output_frames[0]);

  //printPixels(img, "padded.txt");

  // call serial algorithm

  // Calculate the time taken by the serial algorithm
  clock_t t;
  t = clock();
  serial_edge_detection(img, testing_frame);
  t = clock() - t;
  double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

  printf("serial_edge_detection() took %f seconds to execute \n", time_taken); 

  // call cuda
  //run_cuda_edge_detection(img, output_frames[0]);

  //checkResults(testing_frame, output_frames[0]);

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
