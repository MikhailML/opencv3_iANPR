#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "../../include/iANPR.h"


void printHelp (char* fullName)
{
	printf  ("Use: %s <in dir> <out dir> <type number>\n\n", fullName);	
	puts ("type_number: 7 for Russian, 104 for Kazakhstan, 203 for Turkmenistan, 300 for Belarus vehicle registration plates");
	puts ("For more type_numbers please refer to iANPR SDK documentation\n");
	printf ("Example: %s ../../in ../../out/ 7 - recognition of russian vehicle registration plates from ../../in to ../../out\n", fullName);	
}

int main(int argc, char** argv)
{
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
	else if (argc < 4)
	{
		printf ("Too few arguments. For help print %s /?", argv [0]);
		return -2;
	}
    DIR *dir = opendir(argv[1]);
    if(dir)
    {
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

	    struct dirent *ent;		
		char buf[256];

		std::vector <std::string> files;
		while((ent = readdir(dir)) != NULL)
		{
			files.push_back (ent->d_name);
		}

	std::sort (files.begin(), files.end());

	for (int fi = 0; fi < files.size();fi++)
	{
		std::string file = files [fi];
	        puts(file.c_str());
			// Recognition
			strcpy(buf, argv[1]);
			strcat (buf, "/");
			strcat(buf,file.c_str());
			cv::Mat Img = cv::imread (buf, CV_LOAD_IMAGE_COLOR);
			if ( Img.empty() ) continue;
			cv::Mat Gray; 
			cvtColor( Img, Gray, CV_BGR2GRAY );

			strcpy(buf, argv[2]);
			strcat (buf, "/");
			strcat(buf,file.c_str());

			int all = 100;
			CvRect Rects[100];				
			char** res = new char*[all];
			for(int j=0;j<all;j++) res[j] = new char[20];
			

			ANPR_OPTIONS a;		
			a.Detect_Mode = ANPR_DETECTCOMPLEXMODE;
			a.min_plate_size = 500;
			a.max_plate_size = 50000;
			a.max_text_size = 20;		
			a.type_number = atoi (argv[3]);
			a.flags = 2;

			bool isFullType = false;
			for (size_t i = 0; i < anprFullTypesCount; i++)
			if (anprFullTypes[i] == a.type_number)
				isFullType = true;

			IplImage tmpGray = Gray;
			IplImage tmpImg = Img;

			CvRect arearect = cvRect( 0, 0, Gray.cols, Gray.rows );
			
			int i1 = -9999;
			if (isFullType)
				i1 = anprPlateRect (&tmpImg, arearect, a, &all, Rects, res); 
			else
				i1 = anprPlateRect (&tmpGray, arearect, a, &all, Rects, res); 

			printf ("%d\n", all);

			if ( i1 == 0 )
			{						
				for(int j = 0; j < all; j++ )				
				{
					puts (res [j]);				
					if ( strlen( res[j] ) >= 6 )
					{						
						rectangle( Img,cvPoint( Rects[j].x, Rects[j].y),cvPoint(Rects[j].x+Rects[j].width,
							Rects[j].y+Rects[j].height), cvScalar(0,255,255), 2);					
						
						float aa=0.001f*Img.cols;
						int b;

						cv::Size size = cv::getTextSize(res[j], CV_FONT_HERSHEY_SIMPLEX, aa, 1, &b);						
						rectangle( Img, cvPoint(Rects[j].x - 2, Rects[j].y - b*2 - 8 ), cvPoint( Rects[j].x + size.width + 2, 
							Rects[j].y - b*2  + size.height + 6 ), cvScalar( 255, 255, 255 ), CV_FILLED );

						CvPoint pp2,pp1;
						pp2.x=Rects[j].x;
						pp2.y=Rects[j].y;
						pp1.x=Rects[j].x+1;
						pp1.y=Rects[j].y+1;
						cv::putText( Img, res[j], pp1, CV_FONT_HERSHEY_SIMPLEX, aa, cvScalar(0,0,0) );
						cv::putText( Img, res[j], pp2, CV_FONT_HERSHEY_SIMPLEX, aa, cvScalar(0,255,0) );						
					}			
				}
			}
			for(int j=0;j<100;j++) delete [] res[j];
			delete [] res;
				
			cv::imwrite (buf, Img);
			//cvSaveImage( buf, Img);

			//cvReleaseImage( &Img );
			//cvReleaseImage( &Gray );
	    }
	}
	else
	{
	    fprintf(stderr, "Error opening directory\n");
	}
	return 0;
}
