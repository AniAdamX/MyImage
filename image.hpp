// !!! GB2312 Encoded !!!
// 利用了stb_image的三个头文件建立了一个Image类，用以对图像进行读写等操作
// 【作者：AniAdamX】
#pragma once
#ifndef MYIMAGE_IMAGE_H_
#define MYIMAGE_IMAGE_H_

#include <cassert> // assert()函数
#include <cstring> // memcpy()函数

#include <initializer_list>
#include <stdexcept> // logic_error、out_of_range和runtime_error类
#include <memory> // unique_ptr类
#include <string_view>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "include/stb_image_resize2.h"

using std::initializer_list;
using std::logic_error;
using std::memcpy;
using std::out_of_range;
using std::runtime_error;
using std::string_view;
using std::unique_ptr;

void DeleteUCharArr(void* ptr)
{
    delete[] static_cast<unsigned char*>(ptr);
}

// 此处描述类的功能概括和用法例子
// WriteToFile()函数仅支持PNG、TGA、JPG（JPEG）和BMP四种格式且后缀名需小写
class Image {
public:
    // 强枚举类型定义的图像通道类别，其中kRgba中的a意思与kGrayAlpha的Alpha相同，即指alpha通道
    enum class ChannelType {
        kGray = 1, kGrayAlpha, kRgb, kRgba
    };

    // 此处描述类的功能概括和用法例子
    // 仅支持与ChannelType类型对应的通道类别，且每通道仅支持8比特位深（即值只能从0到255）
    class Color {
    public:
        // 构造函数一，使用int类型初始化列表作为参数
        Color(initializer_list<int> init_list)
        {
            // 断言判断初始化列表中值的个数符合要求，且初始化值在有效范围内
            assert(init_list.size() != 0 && init_list.size() < 5);
            for (const auto& element : init_list)
                assert(element >= 0 && element <= 255);

            channel_type_ = static_cast<ChannelType>(init_list.size());
            int index = 0;
            for (const auto& element : init_list)
                value_[index++] = element;
        }
        // 构造函数二，使用unsigned char数组（指针）和通道类型作为参数
        Color(const unsigned char* arr, ChannelType channel_type)
        {
            channel_type_ = channel_type;
            memcpy(value_, arr, static_cast<size_t>(channel_type_));
        }
        // 显式要求复制构造函数、赋值运算符函数和析构函数使用默认合成的版本
        Color(const Color&) = default;
        Color&  operator=(const Color&) = default;
        ~Color() = default;

        // 获取或更改当前Color对象的通道类别
        ChannelType channel_type() const
        { return channel_type_; }
        void set_channel_type(ChannelType channel_type)
        { channel_type_ = channel_type; }
        // 获取或更改颜色值
        const unsigned char* value() const
        { return value_; }
        int value(int index) const
        {
            if (index < 0 || index > (static_cast<int>(channel_type_) - 1))
                throw out_of_range("取颜色值的索引小于零或超过当前对象的通道数目！");
            return static_cast<int>(value_[index]);
        }
        void set_value(initializer_list<int> init_list)
        {
            if (init_list.size() != static_cast<decltype(init_list.size())>(channel_type_))
                throw logic_error("设置颜色值时初始化值不符合当前对象的通道数目！");
            int index = 0;
            for (const auto& element : init_list)
                value_[index++] = element;
        }

    private:
        // 指示当前Color对象的通道类别
        ChannelType channel_type_;
        // 四字节数据数组，需要根据channel_type_的不同来确定哪前几个元素是有效的
        unsigned char value_[4];
    };

    // 构造函数：创建一个指定分辨率和通道类型的空白图像（数值为全零）
    Image(int width, int height, ChannelType channel_type) : width_(width), height_(height), channel_type_(channel_type),
        data_(nullptr, DeleteUCharArr), flag_index_at_bottom_left_(true)
    {
        // 调用构造函数时不应该传入无效值（如果是用户输入则应该在调用构造函数前进行检查处理）
        assert(width_ > 0 && height_ > 0);
        // 无能力处理bad_alloc异常
        data_.reset(new unsigned char[width_ * height_ * static_cast<int>(channel_type_)]());
    }

    // 构造函数：从文件中导入图像
    Image(const char* path_to_file) : data_(nullptr, stbi_image_free), flag_index_at_bottom_left_(true)
    {
        int num_channels;
        data_.reset(stbi_load(path_to_file, &width_, &height_, &num_channels, 0));

        // 如果导入图像失败，无能力处理则就地抛出异常
        if (!data_)
            throw runtime_error(stbi_failure_reason());

        channel_type_ = static_cast<ChannelType>(num_channels);
    }

    // 不希望使用拷贝构造函数和拷贝赋值运算符
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    // 使用默认的析构函数
    ~Image() = default;

    // 将图像数据写入文件，成功返回true
    bool WriteToFile(const char* path_to_file)
    {
        assert(data_);
        // 获取文件后缀名
        string_view str(path_to_file);
        auto suffix_pos_minus_one = str.find_last_of('.');
        if (suffix_pos_minus_one == str.npos)
            return false;
        string_view suffix = str.substr(suffix_pos_minus_one + 1);
        // 依据后缀名调用stb_image_write库的函数
        if (suffix == string_view("png"))
            // 由于图像数据是连续存储的即无特定的内存对齐，因此此处调用时取参数stride_in_bytes = 0使其自动计算
            return static_cast<bool>(stbi_write_png(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
        else if (suffix == string_view("tga"))
            return static_cast<bool>(stbi_write_tga(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
        else if (suffix == string_view("jpg") || suffix == string_view("jpeg"))
            // 最后一个参数是设置JPG格式的压缩质量，0即表示使用库的默认值
            return static_cast<bool>(stbi_write_jpg(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
        else if (suffix == string_view("bmp"))
            return static_cast<bool>(stbi_write_bmp(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
        else
            return false;
    }

    void ResizeInSrgbSpace(int new_width, int new_height)
    {
        assert(data_);
        assert(new_width > 0 && new_height > 0);

        unsigned char* new_data = new unsigned char[new_width * new_height * static_cast<int>(channel_type_)];
        stbir_resize_uint8_srgb(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
            static_cast<stbir_pixel_layout>(channel_type_)); // 返回0表示出现错误
        data_.reset(new_data);
        data_.get_deleter() = DeleteUCharArr;
        width_ = new_width;
        height_ = new_height;
    }

    void ResizeInLinearSpace(int new_width, int new_height)
    {
        assert(data_);
        assert(new_width > 0 && new_height > 0);

        unsigned char* new_data = new unsigned char[new_width * new_height * static_cast<int>(channel_type_)];
        stbir_resize_uint8_linear(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
            static_cast<stbir_pixel_layout>(channel_type_)); // 返回0表示出现错误
        data_.reset(new_data);
        data_.get_deleter() = DeleteUCharArr;
        width_ = new_width;
        height_ = new_height;
    }

    // 部分取值函数
    int width() const
    { return width_; }
    int height() const
    { return height_;}
    ChannelType channel_type() const
    { return channel_type_; }
    // 获取或更改像素索引时原点位置标志量
    bool flag_index_at_bottom_left()  const
    { return flag_index_at_bottom_left_; }
    void set_flag_index_at_bottom_left(bool flag_index_at_bottom_left)
    { flag_index_at_bottom_left_ = flag_index_at_bottom_left; }
    // 获取图像单个像素的颜色
    Color GetPixelColor(int x_index, int y_index) const
    {
        assert(data_);
        if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
            throw out_of_range("在对像素进行索引时超出范围！");
        
        int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + width_)
            : (y_index * width_ + x_index);
        unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
        return Color(pixel_ptr, channel_type_);
    }
    // 更改图像单个像素的颜色
    void SetPixelColor(int x_index, int y_index, const Color& color)
    {
        assert(data_);
        if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
            throw out_of_range("在对像素进行索引时超出范围！");
        if (color.channel_type() != channel_type_)
            throw logic_error("传入的颜色的通道类别与图像的通道类别不同！");

        int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
            : (y_index * width_ + x_index);
        unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
        memcpy(pixel_ptr, color.value(), static_cast<size_t>(channel_type_));
    }
    // 更改全部像素的颜色
    void SetAllPixelColor(const Color& color)
    {
        assert(data_);
        bool temp = flag_index_at_bottom_left_;

        // 设置像素索引原点位于左上角以减少运算量
        flag_index_at_bottom_left_ = false;
        for (int x = 0; x < width_; ++x)
            for (int y = 0; y < height_; ++y)
                SetPixelColor(x, y, color);
        flag_index_at_bottom_left_ = temp;
    }

private:
    // 图像横向像素个数
    int width_;
    // 图像纵向像素个数
    int height_;
    // 图像通道类别
    ChannelType channel_type_;
    // 指向图像数据数组的指针（原始数据图像原点是）
    unique_ptr<unsigned char, void(*)(void*)> data_;
    // 指示在对像素索引时图像原点是否在左下角的标志，默认是true，false意味着图像原点在右上角
    bool flag_index_at_bottom_left_;
};

#endif // MYIMAGE_IMAGE_H_
