#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "../../include/iANPR.h"
#include <stdio.h>


void printHelp (char* fullName)
{
	printf  ("Use: %s <type_number> <path to image>\n\n", fullName);	
	puts ("type_number: 7 for Russian, 104 for Kazakhstan, 203 for Turkmenistan, 300 for Belarus vehicle registration plates");
	puts ("For more type_numbers please refer to iANPR SDK documentation\n");
	printf ("Example: %s 7 C:\\test.jpg - recognition of russian vehicle registration plates from file test.jpg\n", fullName);	
}


int main( int argc, char** argv)
{
	cv::Mat Img; 
	cv::Mat grayImg;	

	// filter input
	if (argc < 2)
	{
		printf ("Too few arguments. For help print %s /?", argv [0]);
		return -1;
	}
	else if (!strcmp (argv [1], "help") || !strcmp (argv [1], "-help") || !strcmp (argv [1], "--help") || !strcmp (argv [1], "/?"))
	{
		printHelp (argv [0]);
		return 0;
	}	
	else if (argc < 3)
	{
		printf ("Too few arguments. For help print %s /?", argv [0]);
		return -2;
	}	
	else // argc >= 3
		Img = cv::imread ( argv[2], CV_LOAD_IMAGE_COLOR  );

	if (Img.empty())
	{
		printf( "Can't load file!\n");
		return -4;
	}
	
	CvRect Rects[100];
	int all = 100;
	char** res = new char*[all];
	for(int j=0;j<all;j++) res[j] = new char[20];
	ANPR_OPTIONS a;
	a.Detect_Mode = ANPR_DETECTCOMPLEXMODE;
	a.min_plate_size = 500;
	a.max_plate_size = 50000;
	a.max_text_size = 20;		
	a.type_number = atoi (argv [1]);		
	a.flags = 0;
	
	bool isFullType = false;
	for (size_t i = 0; i < anprFullTypesCount; i++)
	if (anprFullTypes[i] == a.type_number)
		isFullType = true;

	// Вызов LicenseCapture необходим только для платных версий
	// И только один раз, перед первым распознаванием.	
	char* key = new char[8001]; memset(key, 0, 8001);
	FILE* f = fopen("lic.key", "rb");
	if (f != NULL)
	{
		fread((void*)key, 8000, 1, f);
		fclose(f);
	}
	else
		puts("WARNING! File lic.key not found. This may crash program if you use license version of iANPR SDK dlls");

	LicenseValue(key);
	delete [] key; key = 0;

	IplImage tmpImg = Img;	

	int i = -9999;
	if (isFullType)
		i = anprPlate (&tmpImg,  a, &all, Rects, res );
	else
	{			
		cvtColor (Img, grayImg, CV_BGR2GRAY);

		IplImage tmpGray = grayImg;
		i = anprPlate (&tmpGray,  a, &all, Rects, res);
	}
	
	if (i == 0)
		for (int j = 0; j < all; j++) 
		{				
			printf ("%s\n", res[j]);
		}
	else
		printf( "Error:%d\n", i );

	for(int j=0;j<100;j++) delete [] res[j];
	delete [] res;	

	return 0;
}
