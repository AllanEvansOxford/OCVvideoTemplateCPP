/*Template for openCV video processing
Uses camera or video file for input and
outputs to windows on screen. Trackbars used for parameter control.
This version does the processing sequence
blur->convert to greyscale->gradient->threshold.
Tested with video file from openCV examples at
/opencv/samples/data/vtest.avi
and with webcam input on notebook PC running Lubuntu 15.10
Allan Evans 15 Jan 2017
*/

#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include<iostream>
#include <string>
using namespace std;
using namespace cv;


int const maxGaussianRadius = 20; //parameters for operations
int const maxThreshold = 255;
int gradientScale = 2;
int scharrDelta = 0;
int gradientDepth = CV_16S; //Pixel format for gradient operation result


static void startmessage()//Displayed whenever this program runs
{
    cout<<"OpenCV example of video input , processing, output"<<endl;
    cout<<"Command line arg -h to get help information"<<endl;
}

static void help()//Displayed when -h command line option given
{
    cout<<"Run with no command line args to use camera input"<<endl;
    cout<<"and output to windows on screen only."<<endl;
    cout<<"Add command line arguments -input and/or -output"<<endl;
    cout<<"to input or output results to file."<<endl;
    cout<<"Examples:"<<endl;
    cout<<"./inout1 -input=vtest.avi"<<endl;
    cout<<"./inout1 -input=vtest.avi -output=vtestprocessed.avi"<<endl;
    cout<<"./inout1 -output=camout.avi"<<endl;
}


int main(int argc, char** argv)
{
    double minv,maxv;
    int gaussianRadius=5;   //radius for Gaussian blur
    int intThreshold = 125; //starting value for threshold
    bool outputToFile=false;
    float floatThreshold;
    startmessage();
    VideoCapture cap;
    CommandLineParser parser(argc, argv, "{help h||}{input||}{output||}");
    if (parser.has("help"))
    {
        help();
        return 0;
    }
    string input = parser.get<std::string>("input");
    if (input.empty())//If no input file specified...
        cap.open(0);    //open camera channel
    else
    {
        cap.open(input);//otherwise open input file
        cout <<"Input file:"<<input<<endl;
    }

    string output = parser.get<std::string>("output");//Path to output file

    //get width and height of input for use in output
    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); 
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); 
    int fps = cap.get(CV_CAP_PROP_FPS);
    cout << "Input video size = " << dWidth << "x" << dHeight << endl;
    cout<<"Input video "<<fps<<" FPS "<<endl;
    cout<<"MPEG codec for output file is a bit fussy about frame rate, so we set to 20 in output"<<endl;
    fps=20;
    Size outputSize(static_cast<int> (dWidth), static_cast<int>(dHeight));
    //define output channel (we need to do this even if no output needed
    //just so that the VideoWriter object is defined in code below)
    VideoWriter outputVideoWriter (output, CV_FOURCC('P','I','M','1'), fps, outputSize, false); 
    //CV_FOURCC specifies the codec.Last arg is true for colour, false for greyscale.
    //See http://opencv-srf.blogspot.co.uk/2011/09/saving-images-videos_16.html
    // for some codecs
    if (!output.empty())//If output file specified...
    {
	cout <<"Output file:"<<output<<endl;
	if ( !outputVideoWriter.isOpened() ) //If the videoWriter hasn't started OK...
		cout << "FAILED TO OPEN OUTPUT VIDEO FILE" << endl;
	else
		outputToFile=true;
    }
    

    
    if( !cap.isOpened() )
    {
        cout<<"Can't find video source"<<endl;
        return -1;
    }
    Mat input_image, output_image,gx,gy,g;
    cap >> input_image;
    if(input_image.empty())
    {
        cout<<"Can't read data from the video source"<<endl;
        return -1;
    }
    
    namedWindow("input");
    createTrackbar( "Blur radius:", "input", &gaussianRadius, maxGaussianRadius);
    namedWindow("output");
    createTrackbar( "Threshold:", "output", &intThreshold, maxThreshold);
    
    while(true)
    {
        cap >> input_image;
        if( input_image.empty() )
            break;
	//Gaussian blur input image
	GaussianBlur( input_image, input_image, Size(gaussianRadius*2+1,gaussianRadius*2+1), 0, 0, BORDER_DEFAULT );
	//Convert to greyscale and put in 'outputimage'
	cvtColor( input_image, output_image, CV_BGR2GRAY );
	//output_image=input_image.clone();
	//Sobel-like edge detect
        Scharr(output_image, gx, gradientDepth, 1,0,gradientScale, scharrDelta, BORDER_DEFAULT );
        convertScaleAbs( gx, gx);	
        Scharr( output_image, gy, gradientDepth, 0,1, gradientScale, scharrDelta, BORDER_DEFAULT );	
        convertScaleAbs( gy, gy);
        addWeighted( gx, 0.5, gy, 0.5, 0, g);
	
	//Convert to uchar again
	minMaxLoc(g, &minv, &maxv);
	floatThreshold  = minv + (maxv-minv)*((float) intThreshold/255);
	threshold( g, output_image, floatThreshold, 0, THRESH_TOZERO);
	//last arg specifies mode where we zero all input pixels < threshold
        imshow("input", input_image);
        imshow("output", output_image);
        //cout<<"Max min floatthresh"<< maxv<<" "<< minv<<" "<<floatThreshold <<endl;
	if(outputToFile)
		outputVideoWriter.write(output_image);
	//Exit on ESCAPE key
	//NOTE this slows things down when reading from file (10ms pause for keypress)
        char keycode = (char)waitKey(10);
        if( keycode == 27 )
            break;
    }
    return 0;
}