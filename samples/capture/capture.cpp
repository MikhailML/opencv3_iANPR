#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "../../include/iANPR.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

void printHelp (char* fullName)
{
	printf  ("Use: %s <type_number> <path to video file>\n\n", fullName);	
	puts ("type_number: 7 for Russian, 104 for Kazakhstan, 203 for Turkmenistan, 300 for Belarus vehicle registration plates");
	puts ("For more type_numbers please refer to iANPR SDK documentation\n");
	printf ("Example: %s 7 C:\\test.avi - recognition of russian vehicle registration plates from file test.avi\n", fullName);
	printf ("Example: %s 104 - recognition of kazakh vehicle registration plates from webcam\n", fullName);	
}


int main(int argc, char** argv)
{	
	cv::VideoCapture frameCapture;

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
	else if (argc == 2)		
		frameCapture.open(0);
	else
		frameCapture.open (argv[2]);

	if (!frameCapture.isOpened())
	{
		puts ("Can't load file or camera");
		return -100;
	}
	
	/*char buffer[256];
	sprintf( buffer, "out.avi" );
	CvVideoWriter* cvVideoWriter = 0; 		*/
	
	cv::Mat grayFrame;
	cv::Mat image (800, 600, CV_8UC3);
	
	int i = 0;
	char mem[100][20];
	int all_mem = 0;

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

	for(;;)
    {		
		cv::Mat frame;
		frameCapture >> frame;
		
		if (frame.empty())
            break;

		if (grayFrame.empty())
			grayFrame.zeros(cv::Size(frame.cols, frame.rows), CV_8UC1);
		
		cvtColor( frame, grayFrame, CV_BGR2GRAY );

		int all = 100;
		CvRect Rects[100];				
		char** res = new char*[all];
		for(int j=0;j<all;j++) res[j] = new char[20];
		ANPR_OPTIONS a;		
		a.Detect_Mode = ANPR_DETECTCOMPLEXMODE;
		a.min_plate_size = 500;
		a.max_plate_size = 25000;
		a.max_text_size = 20;		
		a.type_number = atoi (argv [1]);				
		a.flags = 0;

		bool isFullType = false;
		for (size_t i = 0; i < anprFullTypesCount; i++)
		if (anprFullTypes[i] == a.type_number)
			isFullType = true;

		//CvRect arearect = cvRect( 300, 400, 1400, 500);
		cv::Rect arearect (0, 0, grayFrame.cols, grayFrame.rows);
		cv::imwrite( "out.png", grayFrame );
		int i1;

		IplImage tmpFrame = frame;
		IplImage tmpGrayFrame = grayFrame;

		if (isFullType)
			i1 = anprPlateRect (&tmpFrame, arearect, a, &all, Rects, res);
		else
			i1 = anprPlateRect (&tmpGrayFrame, arearect, a, &all, Rects, res);

		printf( "Ret:%d; num:%d; cand:%d\n", i1, i, all);
		if (i1 == 0)
		{						
			for(int j = 0; j < all; j++ )				
			{				
				if ( strlen( res[j] ) >= 1 )
				{
					int k =0;
					for (int j1 = 0; j1 < all_mem; j1++)
						if ( strcmp( res[j], mem[j1]) == 0 ) k = 1;	// Если несколько раз подряд один номер, то считается распознан правильно
					if ( k == 0 ) continue;
					cv::rectangle( frame, cvPoint( Rects[j].x, Rects[j].y),cvPoint(Rects[j].x+Rects[j].width,
						Rects[j].y+Rects[j].height), cvScalar(0, 255,255), 2);
					
					CvFont font;
					float scale = 0.001f * frame.cols;
					
					CvPoint pp2,pp1;

					pp2.x=Rects[j].x;
					pp2.y=Rects[j].y;
					pp1.x=Rects[j].x+1;
					pp1.y=Rects[j].y+1;					

					cv::putText (frame, res[j], pp1, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 0, 0));
					cv::putText (frame, res[j], pp2, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 255, 0));
					std::ofstream out;
					out.open("resault.txt");
					if (out.is_open())
					{
						out << res[j] << std::endl;
					}					
				}			
			}
			// Копирование в память
			for(int j = 0; j < all; j++ )				
			{
				for( i1 = 0; i1 < strlen( res[j] ); i1++ )
					if ( res[j][i1] == '?' ) {
						i1 = -1;
						break;
					}
				if ( i1 == -1 ) continue;
				if ( strlen( res[j] ) >= 1 )
				{
					strcpy( mem[j], res[j] );
				}
			}
			all_mem = all;
		}				
		for(int j=0;j<100;j++) delete [] res[j];
		delete [] res;
		i++;

		cv::resize(frame, image, image.size());
		//cv::imshow( "frame", image);
		/*if (!cvVideoWriter)
		{
				cvVideoWriter=cvCreateVideoWriter( buffer, CV_FOURCC('D', 'I', 'V', '3'), 30,
						cvGetSize( frame) );
		}	
		cvWriteFrame( cvVideoWriter, frame );*/

		int c = cvWaitKey( 20 );
		if ( c== 27 ) break;
	}

	//cvReleaseCapture( &capture );
	//cvReleaseVideoWriter( &cvVideoWriter );

	return 0;
}
