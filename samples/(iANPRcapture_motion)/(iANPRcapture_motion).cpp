#include "opencv2/highgui/highgui.hpp"
#include "../../include/iANPRcapture.h"
#include <stdio.h>
#include <ctime>
#include <list>
#include <limits>
#include <iostream>
#include <fstream>

struct Plate
{	
	std::string str;
	std::string timeStamp;

	cv::Mat img;
	cv::Rect rect;	
	
	cv::Point plateCenter;
	int distToCenterY;

	Plate() : str(""), timeStamp(""), img(), rect(cvRect (0,0,0,0)), plateCenter (cvPoint (0,0)), distToCenterY(INT_MAX) {}
};

void printHelp (char* fullName)
{
	printf  ("Use: %s <type_number> <path to video file> <OPTIONAL listSize> \n\n", fullName);	
	puts ("type_number: 7 for Russian, 104 for Kazakhstan, 203 for Turkmenistan, 300...... for Belarus vehicle registration plates");
	puts ("For more type_numbers please refer to iANPR SDK documentation\n");
	printf ("Example: %s 7 C:\\test.avi - recognition of russian vehicle registration plates from file test.avi\n", fullName);
	printf ("Example: %s 104 - recognition of kazakh vehicle registration plates from webcam\n", fullName);
}


std::string getTimeStamp()
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[512];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d.%m.%Y; %H:%M:%S", timeinfo);
	std::string str(buffer);

	return str;
}

int drawPlate(cv::Mat img, CvRect rect, const char* str)
{
	cv:rectangle(img, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width,
		rect.y + rect.height), CV_RGB(255, 255, 0), 2);
	
	float scale = 0.001f * img.cols;

	cv::Point pp2, pp1;
	pp2.x = rect.x;
	pp2.y = rect.y;
	pp1.x = rect.x + 1;
	pp1.y = rect.y + 1;
	
	cv::putText(img, str, pp1, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 0, 0));
	cv::putText(img, str, pp2, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 255, 0));
					std::ofstream out;
					out.open("resault.txt");
					if (out.is_open())
					{
						out << str << std::endl;
					}
	return 0;
}


bool isInTrajectory(CvPoint* Points, CvPoint pt, int all_points)
{
	for (int j = 0; j < all_points; j++)
		if (Points[j].x == pt.x && Points[j].y == pt.y)
			return true;

	return false;
}


int main(int argc, char** argv)
{	
	cv::VideoCapture capture;
	
	size_t listSize = 50;	
	int controLinesCenterY = -1;
	int minDiffToControlLinesCenter = -1;
	
	// filter input
	if (argc < 2)
	{
		printf ("Too few arguments. For help print %s /?\n", argv [0]);
		return -1;
	}
	else if (!strcmp (argv [1], "help") || !strcmp (argv [1], "-help") || !strcmp (argv [1], "--help") || !strcmp (argv [1], "/?"))
	{
		printHelp (argv [0]);
		return 0;
	}
	else if (argc == 2)		
		capture.open (0);
	else
		capture.open(argv[2]);

	if (!capture.isOpened())
	{
		puts ("Can't load file or camera");
		return -100;
	}

	if (argc > 3)
		listSize = atoi(argv[3]);

	std::list <Plate> plates;
	std::list <Plate>::iterator* optimalPlates = new std::list <Plate>::iterator[listSize];
	
	char buffer[256];
	sprintf( buffer, "out.avi" );
	cv::VideoWriter videoWriter; 		
	ANPR_OPTIONS a;
	a.Detect_Mode = ANPR_DETECTCOMPLEXMODE;
	a.min_plate_size = 500;
	a.max_plate_size = 50000;
	a.max_text_size = 20;		
	a.type_number = atoi (argv [1]);
	iANPRCapture i_capture;

	bool isFullType = false;
	for (size_t i = 0; i < anprFullTypesCount; i++)
	if (anprFullTypes[i] == a.type_number)
		isFullType = true;

	cv::Mat grayFrame;
	cv::Mat image (800, 600, CV_8UC3);	
	int i = 0;	

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

	LicenseCapture(key);
	delete [] key; key = 0;

	CvPoint Lines[4] = {0};
	for(;;)
    {		
		cv::Mat frame;
		capture >> frame;

		if(frame.empty())
            break;

		while (plates.size() > listSize - 1) // Удаляем устаревший кадр
			plates.pop_front();

		if (grayFrame.empty())
		{
			grayFrame.zeros(frame.cols, frame.rows, CV_8UC1);			
            //grayFrame->origin = frame->origin;			
			
			i_capture = CreateiANPRCapture (20, a, cvRect( 0, 0, frame.cols, frame.rows));
			CreateMemoryForiANPRCapture( i_capture, 10, 15, 100 );
			
			Lines[0].x = int (frame.cols * 0.1f); Lines[0].y = int (frame.rows * 0.6f);
			Lines[1].x = int (frame.cols * 0.3f); Lines[1].y = int (frame.rows * 0.6f);
			Lines[2].x = int (frame.cols * 0.1f); Lines[2].y = int (frame.rows * 0.7f);
			Lines[3].x = int (frame.cols * 0.3f); Lines[3].y = int (frame.rows * 0.7f);
			
			CreateLineIntersection (i_capture, Lines[0], Lines[1], Lines[2], Lines[3]);
			controLinesCenterY = (Lines[0].y + Lines[1].y + Lines[2].y + Lines[3].y) / 4;			
		}

		cvtColor (frame, grayFrame, CV_BGR2GRAY);

		int i1 = 0;
		CvRect Rects[100];
		int all = 100;
		char** res = new char*[all];
		for(int j=0;j<all;j++) res[j] = new char[20];

		IplImage tmpFrame = IplImage(frame);
		IplImage tmpGrayFrame = IplImage(grayFrame);

		if (isFullType)
			i1 = AddFrameToiANPRCapture( i_capture, &tmpFrame, &all, Rects, res );
		else 
			i1 = AddFrameToiANPRCapture( i_capture, &tmpGrayFrame, &all, Rects, res );

		//printf( "Ret:%d; num:%d; time:%5.3f  ", i1, i, all );
		
		if (i1 == 0)
		{
			for (int j = 0; j < all; j++)
			{
				if (strlen(res[j]) >= 1)
				{
					printf("%s;", res[j]);

					int plateCenterY = Rects[j].y + Rects[j].height / 2; // Добавляем результат успешного распознавания в список

					Plate tmpPlate;
					tmpPlate.str = res[j];
					tmpPlate.timeStamp = getTimeStamp();

					tmpPlate.rect = Rects[j];
					tmpPlate.distToCenterY = abs(plateCenterY - controLinesCenterY);
					tmpPlate.plateCenter = cvPoint(Rects[j].x + Rects[j].width / 2, Rects[j].y + Rects[j].height / 2);
					tmpPlate.img = frame;

					plates.push_back(tmpPlate);
					drawPlate(frame, Rects[j], res[j]);
				}
			}
		}//if ( i1 == 0 )		
		else // "Сдвигаем" список 
			plates.push_back(Plate());

		printf( "\n" );
		all = 100;
		CvPoint Points[1000];
		int all_points = 1000;
		GetNumbersInMemory( i_capture, &all, res , 20, Points, &all_points );

		if ( all > 0 )
		{
			for (int j = 0; j < all; j++)
			{				
				optimalPlates [j] = plates.end();

				for (std::list <Plate>::iterator l = plates.begin(); l != plates.end(); l++)
					if (optimalPlates[j] == plates.end() || (l->str == res[j] && l->distToCenterY < optimalPlates[j]->distToCenterY && isInTrajectory(Points, l->plateCenter, all_points)))					
						optimalPlates[j] = l;				

				if (optimalPlates[j]->str != res[j])
					printf("listSize is too small. Output is incorrect.\n");
			}

			for(int j = 0; j < all; j++ )
				printf("(%s: %s)\n", optimalPlates[j]->str.c_str(), optimalPlates[j]->timeStamp.c_str());
		}		

		if ( Lines[0].y != 0 )
		{	
			cv::line (frame, Lines[0], Lines[1], CV_RGB(0,255,255), 2);
			cv::line (frame, Lines[2], Lines[3], CV_RGB(0,255,255), 2);
		}

		cv::resize (frame, image, image.size());
		//cv::imshow ("frame", image);		

		if (!videoWriter.isOpened())
			videoWriter.open(buffer, CV_FOURCC('D', 'I', 'V', '3'), 20, cv::Size(frame.cols, frame.rows));
		
		videoWriter.write(frame);

		// Рисование траектории и вывод дополнительного окна
		if ( all > 0 && all_points > 1)
		{
			for(int j = 0; j < all_points; j++ )
			{
				cv::circle (frame, Points[j], 5, CV_RGB(0, 0, 255), 3);
				if (j > 0)
					line (frame, Points[j], Points[j-1], CV_RGB(0,0,255), 2);

				cv::circle (optimalPlates[0]->img, Points[j], 5, CV_RGB(0, 0, 255), 3);
				if (j > 0)
					line (optimalPlates[0]->img, Points[j], Points[j - 1], CV_RGB(0, 0, 255), 2);
			}

			float scale = 0.001f * frame.cols;			
			int b;

			cv::Size size = cv::getTextSize(res[0], CV_FONT_HERSHEY_SIMPLEX, scale, 1, &b);
			cv::rectangle (frame, cvPoint (0, 0), cvPoint( size.width + 2, 50), CV_RGB (255, 255, 255), CV_FILLED);
			
			cv::rectangle(optimalPlates [0]->img, cvPoint(0, 0), cvPoint(size.width + 2,
				50), CV_RGB(255, 255, 255), CV_FILLED);

			CvPoint pp2,pp1;
			pp2.x=0;
			pp2.y=40;
			pp1.x=1;
			pp1.y=41;

			cv::putText( frame, res[0], pp1, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0,0,0) );
			cv::putText( frame, res[0], pp2, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0,255,0) );

			cv::putText(optimalPlates[0]->img, res[0], pp1, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 0, 0));
			cv::putText(optimalPlates[0]->img, res[0], pp2, CV_FONT_HERSHEY_SIMPLEX, scale, CV_RGB(0, 255, 0));

			cv::resize (frame, image, image.size());
			cv::imshow ("frame", image);

			for (int j = 0; j < 60; j++)
				videoWriter.write(frame);

			for (int j = 0; j < all; j++)
			{
				drawPlate(optimalPlates[j]->img, optimalPlates[j]->rect, optimalPlates[j]->str.c_str());
				cv::imwrite("intersectFrame.jpg", optimalPlates[j]->img);
			}

			// Удаляем отработавшие номера из списка
			for (int j = 0; j < 1/*all*/; j++)
			{
				std::list <Plate>::iterator l = plates.begin();
				while (l != plates.end())
				{
					if (l->str == res[j] && isInTrajectory(Points, l->plateCenter, all_points))
					{
						//if (l->img != 0)
						//	cvReleaseImage(&l->img);

						l = plates.erase(l);
					}						
					else
						l++;
				}					
			}
		}				

		for(int j=0;j<100;j++) delete [] res[j];
		delete [] res;
		i++;

		int c = cvWaitKey( 20 );
		if ( c == 32 )
			c = cvWaitKey( 0 ); // pause
		if ( c== 27 ) break;
	}

	if (i_capture != NULL) 
		ReleaseiANPRCapture(&i_capture);

	delete[] optimalPlates;

	return 0;
}
