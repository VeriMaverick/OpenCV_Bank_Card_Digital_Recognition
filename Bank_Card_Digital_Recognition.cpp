//Digital image processing three items: Bank Card Digital Recognition
//Software:                 Visual Studio 2022 17.8.3 Visual Studio Code 1.85.0  & (C++17) & OpenCV 4.5.4
//Creation time:            2023/10/19
//Last modification time:   2023/12/3 10:26:56
//Version:                  2.5.6
//Author:                   WangMingJie
//Student ID:               21003160314

#include    <cstdlib>       
#include    <iostream>
#include    <windows.h>
#include    <opencv2/opencv.hpp>
#include    <opencv2/core/core.hpp>
#include    <opencv2/highgui/highgui.hpp>
#include    <opencv2/imgproc/imgproc.hpp>

using namespace cv  ;                                                                               //opencv namespace
using namespace std ;                                                                               //standard namespace

string TEMPLATE_IMAGE_PATH    = "template.jpg"      ;                                               //template image path
string CARD_IMAGE_PATH        = "card1.jpg"         ;                                               //card image path
string CARD_NUMBER_IMAGE_PATH = "card_number.jpg"   ;                                               //card number image path
string cardNumber             = "1234567890"        ;                                               //Global variable cardNumber

int sel;                                                                                            //Console button switch Select      0: Off 1: On

//Windows API 参数声明
HWND hwndEdit;                                                                                      //声明 hwndEdit 为全局变量  HWND:窗口句柄                 hwndEdit:编辑框句柄
HINSTANCE hInst;                                                                                    //声明 hInst 为全局变量     HINSTANCE:应用程序实例句柄    hInst:应用程序实例句柄
HBRUSH hbrBkgnd = NULL;                                                                             //用于保存画刷的句柄        HBRUSH:画刷句柄               hbrBkgnd:画刷句柄

//  Image loading success/failure function  图像载入成功/失败判断函数
Mat loadImageAndPreprocess(const string& imagePath, const string& imageDescription) 
{
    cout << "...载入" << imageDescription << endl;
    Mat img = imread(imagePath);
    if (img.empty())                                                                                //Determine whether the image was successfully loaded
    {
        throw runtime_error("无法加载图像: " + imagePath);                                           //抛出异常
    }
    else
    {
		cout << "图像载入成功" << endl;
	}
    //imshow(imageDescription, img);                                                                //调试用
    return img;                                                                                     //Return image
}

//  Template contour extraction function    模板轮廓提取函数
void Template_Preprocessing(Mat& src)
{
    Mat gray, edge, dst;
    cvtColor(src, gray, COLOR_BGR2GRAY);                                                            //转换为灰度图像   cvtColor(原图像，目标图像，转换类型)
    GaussianBlur(gray, gray, Size(3, 3), 0, 0);                                                     //高斯滤波  GaussianBlur(原图像，目标图像，高斯核大小，高斯核标准差X，高斯核标准差Y)
    Canny(gray, edge, 50, 150, 3);                                                                  //边缘检测  Canny(原图像，目标图像，低阈值，高阈值，核大小)
    vector<vector<Point>> contours;                                                                 //轮廓 vector<Mat> OpenCV中的数组类
    vector<Vec4i> hierarchy;                                                                        //层次结构
    findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));       //查找轮廓 findContours(原图像，轮廓，层次结构，轮廓检索模式，轮廓近似方法，偏移量)
    dst = Mat::zeros(src.size(), CV_8UC3);                                                          //绘制轮廓
    vector<Rect> rects;                                                                             //矩形块
    for (int i = 0; i < contours.size(); i++)                                                       //遍历轮廓
    {
        Rect rect = boundingRect(contours[i]);                                                      //获取轮廓的矩形块  
        if (rect.width > 10 && rect.height > 10)                                                    //过滤掉过小的矩形块
        {
            rects.push_back(rect);
            drawContours(dst, contours, i, Scalar(255, 255, 255), 1, 1, hierarchy, 1, Point(0));    //绘制轮廓 drawContours (绘制轮廓的图像，轮廓，轮廓索引，颜色，厚度，线型，层次结构，最大层次，偏移量)
            rectangle(dst, rect, Scalar(0, 255, 0), 0);                                             //绘制矩形块     rectangle(绘制矩形块的图像，矩形块，颜色，厚度)
        }
    }
    //imshow("轮廓图像", dst);//调试用
    sort(rects.begin(), rects.end(), [](Rect a, Rect b) {return a.x < b.x; });                      //按矩形块的x坐标排序
    for (int i = 0; i < rects.size(); i++)
    {
        Mat roi = src(rects[i]);                                                                    //获取矩形块对应的图像区域
        Mat gray_roi, binary_roi;
        cvtColor(roi, gray_roi, COLOR_BGR2GRAY);                                                    //转换为灰度图像 cvtColor(原图像，目标图像，转换类型)
        threshold(gray_roi, binary_roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);                   //二值化 threshold(原图像，目标图像，阈值，最大值，阈值类型)
        vector<vector<Point>> contours_roi;
        vector<Vec4i> hierarchy_roi;
        findContours(binary_roi, contours_roi, hierarchy_roi, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));	//查找轮廓
        for (int j = 0; j < contours_roi.size(); j++)
        {
            Rect rect_roi = boundingRect(contours_roi[j]);                                          //获取轮廓的矩形块
            if (rect_roi.width > 100 && rect_roi.height > 200)                                      //过滤掉过小的矩形块
            {
                rectangle(roi, rect_roi, Scalar(255, 255, 255), 0);                                 //绘制矩形块  rectangle(绘制矩形块的图像，矩形块，颜色，厚度)
            }
        }
        //imshow(to_string(i), roi);                                                                //调试用
        imwrite("template"+ to_string(i) + ".jpg", roi);
    }
}

//  Image preprocessing function    图像预处理函数
void ImagePreprocessing(Mat& src)
{
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);                                                            //图像转灰度图
    GaussianBlur(gray, gray, Size(3, 3), 0, 0);                                                     //高斯滤波
    Mat edge;
    Canny(gray, edge, 200, 255, 3);                                                                 //边缘检测
    //imshow("边缘检测", edge);                                                                     //调试用
    imwrite("边缘检测.jpg", edge);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(34, 3));                                 //闭运算  圆形结构元素 
    morphologyEx(edge, edge, MORPH_CLOSE, kernel);                                                  //闭运算
    //imshow("形态操作(闭运算)", edge);                                                              //调试用
    imwrite("闭运算.jpg", edge);
    vector<vector<Point>> contours;                                                                 //轮廓
    vector<Vec4i> hierarchy;                                                                        //层次结构
    findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));       //查找轮廓 findContours(原图像，轮廓，层次结构，轮廓检索模式，轮廓近似方法，偏移量)
    Mat dst = Mat::zeros(src.size(), CV_8UC3);                                                      //绘制轮廓
    vector<Rect> rects;//矩形块
    rects.reserve(contours.size());    
    //遍历轮廓
    for (int i = 0; i < contours.size(); i++)                                                       //遍历轮廓
    {
        Rect rect = boundingRect(contours[i]);                                                      //获取轮廓的矩形块
        rects.push_back(rect);                                                                      //将矩形块添加到矩形块数组中
        if (rect.width > 200 && rect.height > 200)                                                  //过滤掉过小的矩形块
        {
            drawContours(dst, contours, i, Scalar(255, 255, 255), 1, 1, hierarchy, 1, Point(0));    //绘制轮廓 drawContours (绘制轮廓的图像，轮廓，轮廓索引，颜色，厚度，线型，层次结构，最大层次，偏移量)
            rectangle(dst, rect, Scalar(0, 255, 0), 0);                                             //绘制矩形块 rectangle(绘制矩形块的图像，矩形块，颜色，厚度)
        }
    }
    //按矩形块的x坐标排序
    sort(rects.begin(), rects.end(), [](Rect a, Rect b) {return a.x < b.x; });                      //sort(开始位置，结束位置，比较函数)
    //绘制矩形块
    for (int i = 0; i < rects.size(); i++)                                                          //遍历轮廓
    {
        Rect rect = rects[i];                                                                       //获取矩形块
        if (rect.width > 20 && rect.height >20)                                                     //过滤掉过小的矩形块
        {
            rectangle(src, rect, Scalar(0, 255, 0), 0);                                             //绘制矩形块
        }
    }
    int max = 0;                                                                                    //最长的矩形块的宽度
    int index = 0;                                                                                  //最长的矩形块的索引
    for (int i = 0; i < rects.size(); i++)                                                          //找出最长的矩形块
    {
        Rect rect = rects[i];
        if (rect.width > max)                                                                       //如果矩形块的宽度大于max，则将max赋值为矩形块的宽度，index赋值为i
        {
            max = rect.width;                                                                       //max为最长的矩形块的宽度
            index = i;
        }
    }
    //imshow("画框", src);//调试用
    imwrite("画框.jpg", src);
    for (int i = 0; i < rects.size(); i++)                                                          //只保留最长的矩形块，其他矩形块恢复原样
    {
        Rect rect = rects[i];                                                                       //获取矩形块
        if (i != index)                                                                             //如果不是最长的矩形块
        {
            rectangle(src, rect, Scalar(255, 0, 0), 0);                                             //绘制矩形块 rectangle(绘制矩形块的图像，矩形块，颜色，厚度)  用蓝色重新绘制
        }
    }
    Rect rect = rects[index];                                                                       //获取最长的矩形块
    rectangle(src, rect, Scalar(0, 255, 0), 1);                                                     //绘制矩形块
    //imshow("最长的矩形块", src);//调试用
    imwrite("最长的矩形块.jpg", src);
    //获取最长的矩形块对应的图像区域
    Mat roi = src(rect);                                                                            //获取矩形块对应的图像区域
    Mat gray_roi, binary_roi;                                                                       //灰度图像，二值图像
    cvtColor(roi, gray_roi, COLOR_BGR2GRAY);                                                        //转换为灰度图像
    threshold(gray_roi, binary_roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);                       //二值化
    bitwise_not(binary_roi, binary_roi);                                                            //颜色反转
    //保存银行卡号图像
    imwrite("card_number.jpg", binary_roi);
}

//  Card number contour extraction function     卡号轮廓提取函数
void CardNumberContourExtraction(Mat& src)
{
    Mat gray, edge, dst;                                                                            //灰度图像，边缘图像，轮廓图像
    GaussianBlur(src, gray,  Size(3, 3), 0, 0);                                                     //高斯滤波
    Canny(gray, edge, 50, 150, 3);                                                                  //边缘检测
    vector<vector<Point>> contours;                                                                 //轮廓
    vector<Vec4i> hierarchy;                                                                        //层次结构      
    findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));       //查找轮廓
    dst = Mat::zeros(src.size(), CV_8UC3);                                                          //绘制轮廓
    vector<Rect> rects;             						                                        //矩形块  
    for (int i = 0; i < contours.size(); i++)                                                       //遍历轮廓 
    {
		Rect rect = boundingRect(contours[i]);                                                      //获取轮廓的矩形块
        if (rect.width > 10 && rect.height > 10)                                                    //过滤掉过小的矩形块
        {
			rects.push_back(rect);																    //将矩形块添加到矩形块数组中
			drawContours(dst, contours, i, Scalar(255, 255, 255), 1, 1, hierarchy, 1, Point(0));    //绘制轮廓 drawContours (绘制轮廓的图像，轮廓，轮廓索引，颜色，厚度，线型，层次结构，最大层次，偏移量)
			rectangle(dst, rect, Scalar(0, 255, 0), 0);                                             //绘制矩形块
		}
	}
    //imshow("号码轮廓图像", dst);                                                                  //调试用
    imwrite("号码轮廓图像.jpg", dst);
	sort(rects.begin(), rects.end(), [](Rect a, Rect b) {return a.x < b.x; });                      //按矩形块的x坐标排序
    for (int i = 0; i < rects.size(); i++)
    {
		Mat roi = src(rects[i]);                                                                    //获取矩形块对应的图像区域
		Mat gray_roi, binary_roi;
		cvtColor(roi, gray_roi, COLOR_BGR2GRAY);                                                    //转换为灰度图像
		threshold(gray_roi, binary_roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);                   //二值化
		vector<vector<Point>> contours_roi;
		vector<Vec4i> hierarchy_roi;
		findContours(binary_roi, contours_roi, hierarchy_roi, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));	//查找轮廓
        for (int j = 0; j < contours_roi.size(); j++)
        {
			Rect rect_roi = boundingRect(contours_roi[j]);                                          //获取轮廓的矩形块
            if (rect_roi.width > 100 && rect_roi.height > 200)                                      //过滤掉过小的矩形块
            {
				rectangle(roi, rect_roi, Scalar(255, 255, 255), 0);                                 //绘制矩形块
			}
		}
		//imshow(to_string(i+1), roi);                                                              //调试用
		imwrite("number" + to_string(i+1) + ".jpg", roi);
	}
}   

// 模板匹配函数
pair<double, int> templateMatching(Mat& img, Mat& templ)                                            //pair<最大值，最大值位置>
{
    Mat result;
    // 调整模板的大小以匹配待识别图像的大小
    resize(templ, templ, img.size());                                                               //resize(原图像，目标图像，目标图像大小)
    // 使用基于OpenCV的TM_CCORR_NORMED方法进行匹配  归一化相关系数匹配法
    matchTemplate(img, templ, result, TM_CCORR_NORMED);                                             //matchTemplate(原图像，模板图像，结果图像，匹配方法)
    double minVal, maxVal;                                                                          //最小值，最大值
    Point minLoc, maxLoc;                                                                           //最小值位置，最大值位置
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);                                          //minMaxLoc(输入数组，最小值，最大值，最小值位置，最大值位置)
    return make_pair(maxVal, maxLoc.x);                                                             //make_pair(最大值，最大值位置)
}

// 模板匹配函数
string TemplateMatching(int i)
{
    string cardNumber;
    vector<Mat> templates(10);                                                                      //vector<Mat> OpenCV中的数组类
    for (int i = 0; i < 10; i++)
    {
        templates[i] = imread("template" + to_string(i) + ".jpg");                                  //读取模板图像
    }
    vector<Mat> numbers(19);                                                                        //vector<Mat> OpenCV中的数组类
    for (int i = 1; i <= 19; i++)                                                                   //遍历待识别图像小块
    {
        numbers[i - 1] = imread("number" + to_string(i) + ".jpg");                                  //读取待识别图像
    }
    // 对每一个待识别图像进行模板匹配
    for (int i = 0; i < numbers.size(); i++)
    {
        double maxMatch = -1;                                                                       //最佳匹配精度
        int maxTemplateIndex = -1;                                                                  //最佳匹配模板
        // 对每一个模板进行匹配
        for (int j = 0; j < templates.size(); j++)
        {
            pair<double, int> match = templateMatching(numbers[i], templates[j]);                   //模板匹配
            if (match.first > maxMatch)                                                             //如果匹配精度大于最佳匹配精度
            {
                maxMatch = match.first;                                                             //最佳匹配精度
                maxTemplateIndex = j;                                                               //最佳匹配模板
            }
        }
        cout << "Number:" << i + 1 << "\t最佳匹配模板：" << maxTemplateIndex << " \t匹配精度：" << maxMatch << endl;
        cardNumber += to_string(maxTemplateIndex);                                                  //将最佳匹配模板添加到卡号中
        //cout << maxTemplateIndex;  //调试用 打印最佳匹配模板
    }
    cout << "\n该银行卡卡号为: " << cardNumber << endl;
    return cardNumber;
    waitKey(0);
}

// 在图像上放置字符（卡号）函数
void putCardNumberOnImage(cv::Mat &img, std::string cardNumber) 
{
    Point textPosition(img.cols / 8, img.rows / 2);                                                 //设置文本的位置
    Scalar textColor(255, 0, 0);                                                                    //设置文本的颜色
    // 在图像上放置文本                                                      
    putText(img, cardNumber, textPosition, cv::FONT_HERSHEY_SIMPLEX, 1.1, textColor,3);             //putText(绘制文本的图像，文本，文本位置，字体，字体大小，颜色)
}

//银行卡数字识别函数
string Digital_Recognition(string CARD_IMAGE_PATH)
{
    Mat src = loadImageAndPreprocess(TEMPLATE_IMAGE_PATH, "模板图像");                              //读取模板
    Template_Preprocessing(src);                                                                    //模板轮廓提取
    Mat original = loadImageAndPreprocess(CARD_IMAGE_PATH, "银行卡图像");                           //读取银行卡图像
    imwrite("原图.jpg", original);
    Mat CARD = original.clone();                                                                    //克隆银行卡图像
    ImagePreprocessing(CARD);                                                                       //图像预处理       
    Mat card_number = loadImageAndPreprocess(CARD_NUMBER_IMAGE_PATH, "银行卡号图像");               //读取银行卡号图像
    CardNumberContourExtraction(card_number);                                                       //卡号轮廓提取
    cardNumber = TemplateMatching(19);                                                              //模板匹配
    putCardNumberOnImage(original, cardNumber);                                                     //在银行卡图像上放置卡号
    imwrite("识别结果.jpg", original);
    return cardNumber;                                                                              
}

//文件清理函数
void DestroyAllWindows()
{
    for (int i = 0; i < 10; i++)                                                                    //清理模板图像
    {
		remove(("template" + to_string(i) + ".jpg").c_str());
	}
    for (int i = 1; i <= 19; i++)                                                                   //清理号码轮廓图像
    {
		remove(("number" + to_string(i) + ".jpg").c_str());
	}
    //清理文件
    remove("card_number.jpg");
    remove("边缘检测.jpg");
    remove("闭运算.jpg");
    remove("画框.jpg");
    remove("最长的矩形块.jpg");
    remove("号码轮廓图像.jpg");
    remove("识别结果.jpg");
    remove("原图.jpg");
}

//卡号复制函数
void copyToClipboard(const std::string &s)
{
    OpenClipboard(0);                                                                               //打开剪贴板
    EmptyClipboard();                                                                               //清空剪贴板
    HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE,s.size()+1);                                               //分配内存
    if (!hg)                                                                                        //分配内存失败
    {
        CloseClipboard();                                                                           //关闭剪贴板
        return;
    }
    memcpy(GlobalLock(hg),s.c_str(),s.size()+1);                                                    //拷贝数据
    GlobalUnlock(hg);                                                                               //解锁
    SetClipboardData(CF_TEXT,hg);                                                                   //设置剪贴板数据
    CloseClipboard();                                                                               //关闭剪贴板
    GlobalFree(hg);                                                                                 //释放内存
}

/*********************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
//使用Windows API创建窗口

//窗口初始化函数
void InitializeWindow(HWND hwnd, HINSTANCE hInst) 
{
    DestroyAllWindows();                                                                            //文件清理
    // 创建所有的控件
    HWND hwndButton1    = CreateWindow("BUTTON" , "原图"             ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 150, 80, 30, hwnd, (HMENU)1, hInst, NULL); //按钮控件
    HWND hwndButton2    = CreateWindow("BUTTON" , "边缘检测"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 200, 80, 30, hwnd, (HMENU)2, hInst, NULL);
    HWND hwndButton3    = CreateWindow("BUTTON" , "闭运算"           ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 250, 80, 30, hwnd, (HMENU)3, hInst, NULL);
    HWND hwndButton4    = CreateWindow("BUTTON" , "画框"             ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 300, 80, 30, hwnd, (HMENU)4, hInst, NULL);
    HWND hwndButton5    = CreateWindow("BUTTON" , "找最长框"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 350, 80, 30, hwnd, (HMENU)5, hInst, NULL);
    HWND hwndButton6    = CreateWindow("BUTTON" , "区域提取"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 100, 80, 30, hwnd, (HMENU)6, hInst, NULL);
    HWND hwndButton7    = CreateWindow("BUTTON" , "轮廓提取"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 150, 80, 30, hwnd, (HMENU)7, hInst, NULL);
    HWND hwndButton8    = CreateWindow("BUTTON" , "识别结果"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 200, 80, 30, hwnd, (HMENU)8, hInst, NULL);
    HWND hwndButton9    = CreateWindow("BUTTON" , "模板图像"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 100, 80, 30, hwnd, (HMENU)9, hInst, NULL);
    HWND hwndButton1000 = CreateWindow("BUTTON" , "识别卡1"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 100, 80, 30, hwnd, (HMENU)1000, hInst, NULL);
    HWND hwndButton1001 = CreateWindow("BUTTON" , "识别卡2"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 150, 80, 30, hwnd, (HMENU)1001, hInst, NULL);
    HWND hwndButton1002 = CreateWindow("BUTTON" , "识别卡3"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 200, 80, 30, hwnd, (HMENU)1002, hInst, NULL);
    HWND hwndButton1003 = CreateWindow("BUTTON" , "识别卡4"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 250, 80, 30, hwnd, (HMENU)1003, hInst, NULL);
    HWND hwndButton1004 = CreateWindow("BUTTON" , "识别卡5"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 300, 80, 30, hwnd, (HMENU)1004, hInst, NULL);
    HWND hwndButton1005 = CreateWindow("BUTTON" , "识别卡6"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 350, 80, 30, hwnd, (HMENU)1005, hInst, NULL);
    HWND hwndButton1006 = CreateWindow("BUTTON" , "识别卡7"          ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 400, 80, 30, hwnd, (HMENU)1006, hInst, NULL);
    HWND hwndButton1007 = CreateWindow("BUTTON" , "初始化"           ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 550, 80, 30, hwnd, (HMENU)1007, hInst, NULL);
    HWND hwndButton1008 = CreateWindow("BUTTON" , "退出"             ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 550, 80, 30, hwnd, (HMENU)1008, hInst, NULL);
    HWND hwndButton2000 = CreateWindow("BUTTON" , "复制卡号"         ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 550, 80, 30, hwnd, (HMENU)2000, hInst, NULL);
    HWND hwndButton2001 = CreateWindow("BUTTON" , "打开/关闭控制台"  ,   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 165, 500, 150, 30, hwnd, (HMENU)2001, hInst, NULL); 
    HWND hwndStatic0    = CreateWindow("STATIC" , "银行卡号识别系统" ,   WS_CHILD | WS_VISIBLE | SS_CENTER, 100, 10, 300, 20, hwnd, NULL, hInst, NULL);            
    HWND hwndStatic1    = CreateWindow("STATIC" , "该银行卡号为："   ,   WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 50, 150, 20, hwnd, NULL, hInst, NULL);             //CreateWindow(控件类名，控件文本，控件样式，控件位置，控件大小，父窗口句柄，菜单句柄，应用程序实例句柄，指向创建窗口的参数的指针)
    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
    cardNumber = "...系统初始化完成o(*////▽////*)q";                                                //初始化卡号
    SetWindowText(hwndEdit1, cardNumber.c_str());                                                   //设置编辑框的文本
}

//窗口过程函数
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)               
{
    static HBRUSH hbrBkgnd = NULL;                                                                  //用于保存画刷的句柄
    // 处理窗口事件
    switch( msg )
    {
        // 设置窗口的最小和最大尺寸
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* minMaxInfo = (MINMAXINFO*)lparam;                                           //MINMAXINFO结构体指针
            minMaxInfo->ptMinTrackSize.x = 500;                                                     //设置窗口的最小宽度    500
            minMaxInfo->ptMinTrackSize.y = 700;                                                     //设置窗口的最小高度    700
            minMaxInfo->ptMaxTrackSize.x = 500;                                                     //设置窗口的最大宽度    500
            minMaxInfo->ptMaxTrackSize.y = 700;                                                     //设置窗口的最大高度    700
            return 0;                               
        }  
        // 设置窗口的背景颜色                        
        case WM_ERASEBKGND:                             
        {                               
            RECT rect;                                                                              // 定义一个矩形结构体
            GetClientRect(hwnd, &rect);                                                             // 获取窗口的客户区域    
            if (hbrBkgnd == NULL)                                                                   // 如果画刷句柄为空
            {                               
                hbrBkgnd = CreateSolidBrush(RGB(50, 55, 255));                                      // 设置窗口的背景颜色
            }                               
            FillRect((HDC)wparam, &rect, hbrBkgnd);                                                 // 填充背景颜色
            return 1;
        }
        // 创建菜单
        case WM_CREATE:
        {
            HMENU hMenu, hSubMenu01,hSubMenu02,hSubMenu03,hSubMenu04;                               //菜单，子菜单01，子菜单02，子菜单03，子菜单04
            hMenu = CreateMenu();                                                                   //创建菜单
            //菜单  文件
            hSubMenu01 = CreatePopupMenu();                                                         //创建子菜单
            AppendMenu(hSubMenu01, MF_STRING, 1000, "Recognition card 1");                          //添加菜单项
            AppendMenu(hSubMenu01, MF_STRING, 1001, "Recognition card 2");
            AppendMenu(hSubMenu01, MF_STRING, 1002, "Recognition card 3");
            AppendMenu(hSubMenu01, MF_STRING, 1003, "Recognition card 4");
            AppendMenu(hSubMenu01, MF_STRING, 1004, "Recognition card 5");
            AppendMenu(hSubMenu01, MF_STRING, 1005, "Recognition card 6");
            AppendMenu(hSubMenu01, MF_STRING, 1006, "Recognition card 7");
            AppendMenu(hSubMenu01, MF_STRING, 1007, "Initialize");
            AppendMenu(hSubMenu01, MF_STRING, 1008, "Exit");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu01, "File");                      //将子菜单添加到菜单
            //菜单  工具
            hSubMenu02 = CreatePopupMenu();                                                         //创建子菜单
            AppendMenu(hSubMenu02, MF_STRING, 2000, "Copy card number");
            AppendMenu(hSubMenu02, MF_STRING, 2001, "Open / Close the console");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu02, "Tools");
            //菜单  外观
            hSubMenu03 = CreatePopupMenu();
            AppendMenu(hSubMenu03, MF_STRING, 3000, "Blue");
            AppendMenu(hSubMenu03, MF_STRING, 3001, "Gray");
            AppendMenu(hSubMenu03, MF_STRING, 3002, "Green");
            AppendMenu(hSubMenu03, MF_STRING, 3003, "Red");
            AppendMenu(hSubMenu03, MF_STRING, 3004, "Yellow");
            AppendMenu(hSubMenu03, MF_STRING, 3005, "White");
            AppendMenu(hSubMenu03, MF_STRING, 3006, "Purple");
            AppendMenu(hSubMenu03, MF_STRING, 3007, "Black");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu03, "Appearance");
            //菜单  帮助
            hSubMenu04 = CreatePopupMenu();
            AppendMenu(hSubMenu04, MF_STRING, 4000, "Auther");
            AppendMenu(hSubMenu04, MF_STRING, 4001, "About [Bank Card Digital Recognition]");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu04, "Help");
            SetMenu(hwnd, hMenu);                                                                   //将菜单添加到窗口
        }   break;
        // 处理窗口的关闭事件
        case WM_COMMAND:
            switch( LOWORD(wparam) )
            {
                case 1: //判断原图是否存在
                {   
                    Mat src = imread("原图.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "原图不存在！", "提示", MB_OK|MB_ICONWARNING);             //弹出消息框
                    else
                        imshow("原图.jpg", src);
                }   break;
                case 2: 
                {   //判断边缘检测是否存在
                    Mat src = imread("边缘检测.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "边缘检测不存在！", "提示", MB_OK|MB_ICONWARNING);         //弹出消息框
                    else
                        imshow("边缘检测.jpg", src);
                }   break;
                case 3: 
                {   //判断闭运算是否存在
                    Mat src = imread("闭运算.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "闭运算不存在！", "提示", MB_OK|MB_ICONWARNING);           //弹出消息框
                    else
                        imshow("闭运算.jpg", src);
                }   break;
                case 4: 
                {   //判断画框是否存在
                    Mat src = imread("画框.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "画框不存在！", "提示", MB_OK|MB_ICONWARNING);             //弹出消息框
                    else
                        imshow("画框.jpg", src);
                }   break;
                case 5: 
                {   //判断最长的矩形块是否存在
                    Mat src = imread("最长的矩形块.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "最长的矩形块不存在！", "提示", MB_OK|MB_ICONWARNING);     //弹出消息框
                    else
                        imshow("最长的矩形块.jpg", src);
                }   break;
                case 6: 
                {   //判断区域提取是否存在
                    Mat src = imread("card_number.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "区域提取不存在！", "提示", MB_OK|MB_ICONWARNING);         //弹出消息框
                    else
                        imshow("card_number.jpg", src);
                }   break;
                case 7: 
                {   //判断轮廓提取是否存在
                    Mat src = imread("号码轮廓图像.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "轮廓提取不存在！", "提示", MB_OK|MB_ICONWARNING);    //弹出消息框
                    else
                        imshow("号码轮廓图像.jpg", src);
                }   break;
                case 8: 
                {   //判断识别结果是否存在
                    Mat src = imread("识别结果.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "识别结果不存在！", "提示", MB_OK|MB_ICONWARNING);    //弹出消息框
                    else
                        imshow("识别结果.jpg", src);
                }   break;
                case 9: 
                {   //判断模板图像是否存在
                    Mat src = imread("template.jpg");
                    if (src.empty())
                        MessageBox(hwnd, "模板图像不存在！", "提示", MB_OK|MB_ICONWARNING);    //弹出消息框
                    else
                        imshow("template.jpg", src);
                }   break;

                case 1000:      //识别卡1
                {
                    string CARD_IMAGE_PATH = "card1.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框 MessageBox(父窗口句柄，消息内容，标题，消息框类型)
                }   break;
                case 1001:      //识别卡2
                {
                    string CARD_IMAGE_PATH = "card2.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1002:      //识别卡3
                {
                    string CARD_IMAGE_PATH = "card3.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1003:      //识别卡4
                {
                    string CARD_IMAGE_PATH = "card4.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1004:      //识别卡5
                {
                    string CARD_IMAGE_PATH = "card5.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1005:      //识别卡6
                {
                    string CARD_IMAGE_PATH = "card6.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1006:      //识别卡7
                {
                    string CARD_IMAGE_PATH = "card7.jpg";
                    cardNumber = Digital_Recognition(CARD_IMAGE_PATH);
                    HWND hwndEdit1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, 180, 50, 250, 20, hwnd, NULL, hInst, NULL);
                    SetWindowText(hwndEdit1, cardNumber.c_str());
                    MessageBox(hwnd, "识别成功！o(￣┰￣*)ゞ", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                } break;
                case 1007: //复位
                {
                    destroyAllWindows();                            //关闭所有图片
                    InitializeWindow(hwnd, hInst);  		        //重新初始化窗口
                    DestroyAllWindows();                            //文件清理
                    MessageBox(hwnd, "初始化完成！\(￣︶￣*\))", "提示", MB_OK|MB_ICONASTERISK);   
                }break;
                case 1008:      //退出
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    destroyAllWindows();                                        //关闭所有图片
                    PostQuitMessage(0);                                         //退出消息循环       
                    DestroyAllWindows();                                        //文件清理   
                    exit(0);                                                    //退出程序  
                } break;
                case 2000:      //复制卡号
                {
                    copyToClipboard(cardNumber);
                    MessageBox(hwnd, "\(￣^￣*\))已将卡号复制到剪切板！", "提示", MB_OK|MB_ICONINFORMATION);    //弹出消息框
                }   break;
                case 2001: 
                { 
                    sel = ~sel;                                                 //取反          
                    if(sel)                                                     //打开控制台
                    {
                        ShowWindow(GetConsoleWindow(), SW_SHOW);                    //隐藏控制台窗口 ShowWindow(窗口句柄，显示方式)   SW_HIDE 隐藏窗口   SW_SHOW 显示窗口
                        MessageBox(hwnd, "控制台成功打开！", "提示", MB_OK|MB_ICONWARNING);    //弹出消息框
                    }
                    else    //关闭控制台
                    {
                        ShowWindow(GetConsoleWindow(), SW_HIDE);                    //隐藏控制台窗口 ShowWindow(窗口句柄，显示方式)   SW_HIDE 隐藏窗口   SW_SHOW 显示窗口
                        MessageBox(hwnd, "控制台成功关闭！", "提示", MB_OK|MB_ICONWARNING);    //弹出消息框
                    }
                }   break;
                case 3000:  //修改背景颜色  蓝色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(50, 55, 255));             // 设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3001:  //修改背景颜色  灰色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(128, 128, 128));           // 设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3002:  //修改背景颜色  绿色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(0, 255, 0));               // 设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3003:  //修改背景颜色  红色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(255, 0, 0));                //设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3004:  //修改背景颜色  黄色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(255, 255, 0));              //设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3005:  //修改背景颜色  白色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));            //设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3006:  //修改背景颜色  紫色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(128, 0, 255));              //设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 3007:  //修改背景颜色  黑色
                {
                    if (hbrBkgnd)
                    {
                        DeleteObject(hbrBkgnd);                                 //删除背景颜色
                    }
                    hbrBkgnd = CreateSolidBrush(RGB(0, 0, 0));                  //设置窗口的背景颜色
                    InvalidateRect(hwnd, NULL, TRUE);                           //刷新窗口
                } break;
                case 4000:  MessageBox(hwnd, "大连东软信息学院\n智能与电子工程学院\n微电子工程系\n集成电路设计与集成系统\n作者：集成电路21003班  王明杰\n学号：21003160314", "作者", MB_OK|MB_ICONASTERISK);   break;
                case 4001:  MessageBox(hwnd, "数字图像处理三级项目：银行卡数字识别\nVersion:  2.5.6\nLast modification time:  2023/12/3 10:26:56\n版权所有(C)2023 WangMingJie。\n保留所有权利。\n警告: 本计算机程序受著作权法和国际条约保护。如未经授权而擅自复制或传播本程序(或其中任何部分)，将受到严厉的民事和刑事制裁，并将在法律许可的最大限度内受到起诉。", "关于", MB_OK|MB_ICONASTERISK);   break; 
                default:    break;
             }  break;
        // 处理窗口的关闭事件
        case WM_CLOSE:
            if (hbrBkgnd)
            {
                DeleteObject(hbrBkgnd);                                         //删除背景颜色
            }
            PostQuitMessage(0);                                                 //退出消息循环
            break;
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);                    //默认消息处理函数
    }
    return 0;
}

//WinMain函数
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)                             //WinMain(应用程序实例句柄，应用程序实例句柄，命令行参数，显示方式)
{
    const char CLASS_NAME[] = "Bank Card Digital Recognition V2.5.6";                           //窗口类名
    WNDCLASS wc = { };                                                                          //窗口类
    wc.lpfnWndProc = WindowProcedure;                                                           //窗口过程函数
    wc.hInstance = hInst;                                                                       //应用程序实例句柄
    wc.lpszClassName = CLASS_NAME;                                                              //窗口类名
    if(!RegisterClass(&wc))                                                                     //注册窗口类
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);  //弹出消息框
        return 0;
    }
    HWND hwnd = CreateWindow(CLASS_NAME, "Bank Card Digital Recognition V2.5.6", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, hInst, NULL);//CreateWindow(窗口类名，窗口标题，窗口样式，窗口位置，窗口大小，父窗口句柄，菜单句柄，应用程序实例句柄，指向创建窗口的参数的指针)
    if(hwnd == NULL)                                                                            //创建窗口失败
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);      //弹出消息框
        return 0;
    }
    InitializeWindow(hwnd, hInst);                                                              //初始化窗口
    ShowWindow(hwnd, nCmdShow);                                                                 //显示窗口
    UpdateWindow(hwnd);                                                                         //更新窗口
    MSG msg = { };                                                                              //消息
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);                                                                 //翻译消息
        DispatchMessage(&msg);                                                                  //分发消息
    }
    return 0;
}

/*********************************************************************************************************************************************************************************************************************************************************************************************************************************************************/

//  main function   主函数  程序入口
int main()     
{
    system("mode con cols=80 lines=28");                                                        //改变控制台窗口大小
    system("color 1F");                                                                         //改变控制台窗口颜色
    ShowWindow(GetConsoleWindow(), SW_HIDE);                                                    //隐藏控制台窗口 ShowWindow(窗口句柄，显示方式)   SW_HIDE 隐藏窗口   SW_SHOW 显示窗口
    try                                                                                         //异常处理
    {
        WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);                                    //WinMain(应用程序实例句柄，应用程序实例句柄，命令行参数，显示方式)
    } 
    catch (const exception& e)                                                                  //捕获异常
    {
        cerr << e.what() << endl;                                                               //输出错误信息
    }
    return 0;
}