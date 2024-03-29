#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <string.h>
#include <time.h>
//#include <conio.h>
#define MAX_SOURCE_SIZE (0x100000)

int main(void)
{
	time_t start,end;
	start = clock();
	char tempstr[7 ];

	int i;
	int n;
	printf("Enter n\n");
	scanf("%d",&n);
	scanf("%s",tempstr );
	int len = strlen(tempstr);
	
	 char *str = (char*)malloc(sizeof(char)*len);

	 strcpy(str,tempstr);

	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("q1.cl", "r");
	if(!fp)
	{
		fprintf(stderr, "Failed to load kernel.\n");
		getchar();
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);

	fclose(fp);

	// Get platform & device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, &ret_num_devices);

	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);

	// Create memory buffers on the device for each vector A, B & C
	cl_mem s_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, len * sizeof(char), NULL, &ret);
	cl_mem t_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, (len*n) * sizeof(char), NULL, &ret);


	// Copy lists A & B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, s_mem_obj, CL_TRUE, 0, len * sizeof(char),str, 0, NULL, NULL);
	//ret = clEnqueueWriteBuffer(command_queue, t_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(int), B, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t *)&source_size, &ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	// Create the OpenCL kernel object
	cl_kernel kernel = clCreateKernel(program, "rptstr", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&s_mem_obj); 
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&t_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(int), (void *)&n);
	ret = clSetKernelArg(kernel, 3, sizeof(int), (void *)&len);
	//ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);

	// Execute OpenCL kernel on array
	size_t global_item_size = len;
	size_t local_item_size = 1;

	// Execute kernel on the device
	cl_event event;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
	time_t stime = clock();
	ret = clFinish(command_queue);

	cl_ulong time_start,time_end;
	double total_time;
	clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_START,sizeof(time_start),&time_start,NULL);

	clGetEventProfilingInfo(event,CL_PROFILING_COMMAND_END,sizeof(time_end),&time_end,NULL);
	total_time = (double)(time_end-time_start);

	char *strres = (char*)malloc(sizeof(char)*(len*n));


	// Read memory buffer C on the device to the local variable C
	//int *C = (int *)malloc(sizeof(int)* LIST_SIZE);
	ret= clEnqueueReadBuffer(command_queue, t_mem_obj, CL_TRUE, 0, (len*n) * sizeof(char), strres, 0, NULL, NULL);

	// Display result to the screen
	//for(i = 0; i < LIST_SIZE; i++)
		//printf("%d + %d = %d\n", A[i], B[i], C[i]);

	printf("\n Done");
	strres[len*n]='\0';
	printf("\nResultant  %s",strres);
	


	// Clean up
	ret = clFlush(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(s_mem_obj);
	ret = clReleaseMemObject(t_mem_obj);
//	ret = clReleaseMemObject(c_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	end = clock();
	printf("\n\n Time taken to execute the Kernel in ms = %0.3fmsec\n\n",total_time/1000000);
	printf("\n time taken to execute the whole program in seconds : %0.3f Seconds\n",(end-start)/(double)CLOCKS_PER_SEC );


	free(str);
	free(strres);
	
	
	
	return 0;
}
