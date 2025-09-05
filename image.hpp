//   Note: This file is encoded in GB2312!
//   该文件声明了基于 stb_image 系列库的用于基础图像处理的 Image 类及其内部辅助类 ChannelType 和 Color。其
// 支持在内存中新建空白图像或者从文件中读取图像，支持 1~4 通道（灰度、灰度+Alpha、RGB、RGBA）的每通道 8 bit
// 的图像数据，提供单个像素的读取与设置（含原点在左下或左上两种索引方式）、整图着色、图像写出（png/tga/jpg/bmp
// 格式）以及在 sRGB 或线性空间中进行尺寸缩放等接口。具体实现放在了 image.cpp 中。【作者：AniAdamX】
#pragma once
#ifndef MYIMAGE_IMAGE_H_
#define MYIMAGE_IMAGE_H_

#include <initializer_list>
#include <memory> // 需要使用：unique_ptr类

//   Image类，封装了 1~4 通道（灰度/灰度+Alpha/RGB/RGBA）且 8 bit 每通道的数字图像，支持图像读写、像素
// 访问与尺寸缩放等常用操作。
//   此处需要给出类的用法例子。
class Image {
public:
    // 强枚举类型定义的图像通道类别，其中kRgba中的a意思与kGrayAlpha的Alpha相同，即指alpha通道
    enum class ChannelType {
        kGray = 1, kGrayAlpha, kRgb, kRgba
    };

    //   Color类，用于描述单个像素的颜色值，仅支持与ChannelType类型对应的通道类别，且每通道仅支持8比特位深
    // （即值只能从0到255）。
    //   此处需要给出类的用法例子。
    class Color {
    public:
        // 构造函数一：使用 int 类型初始化列表作为接收参数（元素个数决定通道类型 1~4，值范围 0~255）
        Color(std::initializer_list<int> init_list);
        // 构造函数二：从 unsigned char 数组与通道类型构造，数组中每个元素需存放着八比特的通道数据
        Color(const unsigned char* arr, ChannelType channel_type);
        // 显式要求复制构造函数、赋值运算符函数和析构函数使用默认合成的版本
        Color(const Color&) = default;
        Color&  operator=(const Color&) = default;
        ~Color() = default;

        // 获取或更改当前 Color 对象的通道类别
        ChannelType channel_type() const
        { return channel_type_; }
        void set_channel_type(ChannelType channel_type)
        { channel_type_ = channel_type; }
        // 获取或更改颜色值：value() 返回内部字节数组的首地址，每字节代表一个通道，注意有效数据与当前对象的通
        // 道类别对应；value(i) 访问第 i 通道
        const unsigned char* value() const
        { return value_; }
        int value(int index) const;
        // 设置颜色值：初始化列表长度需等于当前通道数
        void set_value(std::initializer_list<int> init_list);

    private:
        // 指示当前Color对象的通道类别
        ChannelType channel_type_;
        // 四字节数据数组，需要根据channel_type_的不同来确定哪前几个元素是有效的
        unsigned char value_[4];
    };

    // 构造函数一：创建一个指定分辨率和通道类型的空白图像（数值为全零）
    // 参数：width/height 横向或纵向的像素个数；channel_type 通道类型
    Image(int width, int height, ChannelType channel_type);
    // 构造函数二：从文件中导入图像（自动确定通道数）
    // 说明：内部在导入失败时抛出 std::runtime_error 并携带失败原因
    Image(const char* path_to_file);

    // 不希望使用拷贝构造函数和拷贝赋值运算符
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    // 使用默认的析构函数
    ~Image() = default;

    // 将图像数据写入文件，成功返回 true
    // 支持格式：png、tga、jpg/jpeg、bmp（后缀需小写）
    bool WriteToFile(const char* path_to_file);

    // 图像缩放（sRGB 空间）
    void ResizeInSrgbSpace(int new_width, int new_height);
    // 图像缩放（线性空间）
    void ResizeInLinearSpace(int new_width, int new_height);

    // 部分取值函数
    int width() const
    { return width_; }
    int height() const
    { return height_; }
    ChannelType channel_type() const
    { return channel_type_; }
    // 获取或更改像素索引时原点位置标志量（true是左下角，false是左上角）
    bool flag_index_at_bottom_left() const
    { return flag_index_at_bottom_left_; }
    void set_flag_index_at_bottom_left(bool flag_index_at_bottom_left)
    { flag_index_at_bottom_left_ = flag_index_at_bottom_left; }
    // 获取图像单个像素的颜色
    Color GetPixelColor(int x_index, int y_index) const;
    // 更改图像单个像素的颜色
    void SetPixelColor(int x_index, int y_index, const Color& color);
    // 更改全部像素的颜色
    void SetAllPixelColor(const Color& color);

private:
    // 图像横向像素个数
    int width_;
    // 图像纵向像素个数
    int height_;
    // 图像通道类别
    ChannelType channel_type_;
    // 指向图像数据数组的指针
    std::unique_ptr<unsigned char, void(*)(void*)> data_;
    // 指示在对像素索引时图像原点是否在左下角的标志，默认是true，false意味着图像原点在右上角
    bool flag_index_at_bottom_left_;
};

#endif // MYIMAGE_IMAGE_H_
