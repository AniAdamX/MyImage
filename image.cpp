//   Note: This file is encoded in GB2312!
//   本源文件包含 Image 类及其内部 Color 类的具体实现。（与头文件分离实现可以减少编译开销，并避免重复定义第三
// 方库实现宏。
#include "image.hpp"

#include <cassert>
#include <cstring> // 需要使用：memcpy()函数
#include <stdexcept> // 需要使用： out_of_range、logic_error和runtime_error类
#include <string_view>

// 启用 stb 的用户级错误消息，以便在失败时获取更友好的错误描述
#define STBI_FAILURE_USERMSG
// 将 stb 的实现宏放在 .cpp 中，避免被多个翻译单元重复定义（注意以下类似）
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "include/stb_image_resize2.h"

// 删除 new[] 分配的 unsigned char 数组，以下实现配合unique_ptr的自定义删除器进行使用
void DeleteUCharArr(void* ptr)
{
    delete[] static_cast<unsigned char*>(ptr);
}

// ---------------- Image::Color ----------------
// 构造函数一：使用初始化列表设置颜色，元素数量决定通道类型（1~4）
Image::Color::Color(std::initializer_list<int> init_list)
{
    assert(init_list.size() != 0 && init_list.size() < 5);
    for (const auto& element : init_list)
        assert(element >= 0 && element <= 255);

    channel_type_ = static_cast<ChannelType>(init_list.size());
    int index = 0;
    for (const auto& element : init_list)
        value_[index++] = static_cast<unsigned char>(element);
}

// 构造函数二：用于从存放图像数据的字节数组中按指定通道类型拷贝颜色值
Image::Color::Color(const unsigned char* arr, ChannelType channel_type)
{
    channel_type_ = channel_type;
    std::memcpy(value_, arr, static_cast<size_t>(channel_type_));
}

int Image::Color::value(int index) const
{
    if (index < 0 || index > (static_cast<int>(channel_type_) - 1))
        throw std::out_of_range("取颜色值的索引小于零或超过当前对象的通道数目！");
    return static_cast<int>(value_[index]);
}

void Image::Color::set_value(std::initializer_list<int> init_list)
{
    if (init_list.size() != static_cast<decltype(init_list.size())>(channel_type_))
        throw std::logic_error("设置颜色值时初始化值不符合当前对象的通道数目！");
    int index = 0;
    for (const auto& element : init_list)
        value_[index++] = static_cast<unsigned char>(element);
}

// ---------------- Image ----------------
// 构造空白图像：按给定分辨率与通道类型分配并清零数据
Image::Image(int width, int height, ChannelType channel_type)
    : width_(width), height_(height), channel_type_(channel_type),
      data_(nullptr, DeleteUCharArr), flag_index_at_bottom_left_(true)
{
    assert(width_ > 0 && height_ > 0);
    data_.reset(new unsigned char[static_cast<size_t>(width_) * static_cast<size_t>(height_) * static_cast<size_t>(channel_type_)]());
}

// 从文件加载图像：自动检测通道数
Image::Image(const char* path_to_file)
    : data_(nullptr, stbi_image_free), flag_index_at_bottom_left_(true)
{
    int num_channels;
    data_.reset(stbi_load(path_to_file, &width_, &height_, &num_channels, 0));
    if (!data_)
        throw std::runtime_error(stbi_failure_reason());
    channel_type_ = static_cast<ChannelType>(num_channels);
}

// 将图像写入文件：支持 png/tga/jpg(jpeg)/bmp，后缀需小写
bool Image::WriteToFile(const char* path_to_file)
{
    assert(data_);
    std::string_view str(path_to_file);
    auto suffix_pos_minus_one = str.find_last_of('.');
    if (suffix_pos_minus_one == str.npos)
        return false;
    std::string_view suffix = str.substr(suffix_pos_minus_one + 1);

    if (suffix == std::string_view("png"))
        // stride_in_bytes = 0 表示按通道数自动计算行跨度（数据连续存储）
        return static_cast<bool>(stbi_write_png(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
    else if (suffix == std::string_view("tga"))
        return static_cast<bool>(stbi_write_tga(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
    else if (suffix == std::string_view("jpg") || suffix == std::string_view("jpeg"))
        // 最后一个参数为 JPG 质量（0 使用库默认值）
        return static_cast<bool>(stbi_write_jpg(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
    else if (suffix == std::string_view("bmp"))
        return static_cast<bool>(stbi_write_bmp(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
    else
        return false;
}

// 在 sRGB 空间缩放
void Image::ResizeInSrgbSpace(int new_width, int new_height)
{
    assert(data_);
    assert(new_width > 0 && new_height > 0);
    unsigned char* new_data = new unsigned char[static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channel_type_)];
    stbir_resize_uint8_srgb(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
        static_cast<stbir_pixel_layout>(channel_type_));
    // 将 data_ 的删除器切回自定义数组删除器
    data_.reset(new_data);
    data_.get_deleter() = DeleteUCharArr;
    width_ = new_width;
    height_ = new_height;
}

// 在线性空间缩放
void Image::ResizeInLinearSpace(int new_width, int new_height)
{
    assert(data_);
    assert(new_width > 0 && new_height > 0);
    unsigned char* new_data = new unsigned char[static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channel_type_)];
    stbir_resize_uint8_linear(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
        static_cast<stbir_pixel_layout>(channel_type_));
    data_.reset(new_data);
    data_.get_deleter() = DeleteUCharArr; // 删除器需要与数据配套
    width_ = new_width;
    height_ = new_height;
}

// 获取单个像素的颜色：根据原点标志从左下或左上开始计算
Image::Color Image::GetPixelColor(int x_index, int y_index) const
{
    assert(data_);
    if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
        throw std::out_of_range("在对像素进行索引时超出范围！");

    int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
        : (y_index * width_ + x_index);
    unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
    return Color(pixel_ptr, channel_type_);
}

// 设置单个像素的颜色：颜色的通道类型需与图像一致
void Image::SetPixelColor(int x_index, int y_index, const Color& color)
{
    assert(data_);
    if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
        throw std::out_of_range("在对像素进行索引时超出范围！");
    if (color.channel_type() != channel_type_)
        throw std::logic_error("传入的颜色的通道类别与图像的通道类别不同！");

    int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
        : (y_index * width_ + x_index);
    unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
    std::memcpy(pixel_ptr, color.value(), static_cast<size_t>(channel_type_));
}

// 设置全部像素的颜色（此处临时将原点设为左上角以减少运算量）
void Image::SetAllPixelColor(const Color& color)
{
    assert(data_);
    if (color.channel_type() != channel_type_)
        throw std::logic_error("传入的颜色的通道类别与图像的通道类别不同！");

    bool temp = flag_index_at_bottom_left_;
    flag_index_at_bottom_left_ = false;
    for (int x = 0; x < width_; ++x)
        for (int y = 0; y < height_; ++y)
            SetPixelColor(x, y, color);
    flag_index_at_bottom_left_ = temp;
}
