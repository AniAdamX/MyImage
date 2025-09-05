//   Note: This file is encoded in GB2312!
//   ��Դ�ļ����� Image �༰���ڲ� Color ��ľ���ʵ�֡�����ͷ�ļ�����ʵ�ֿ��Լ��ٱ��뿪�����������ظ��������
// ����ʵ�ֺꡣ
#include "image.hpp"

#include <cassert>
#include <cstring> // ��Ҫʹ�ã�memcpy()����
#include <stdexcept> // ��Ҫʹ�ã� out_of_range��logic_error��runtime_error��
#include <string_view>

// ���� stb ���û���������Ϣ���Ա���ʧ��ʱ��ȡ���ѺõĴ�������
#define STBI_FAILURE_USERMSG
// �� stb ��ʵ�ֺ���� .cpp �У����ⱻ������뵥Ԫ�ظ����壨ע���������ƣ�
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "include/stb_image_resize2.h"

// ɾ�� new[] ����� unsigned char ���飬����ʵ�����unique_ptr���Զ���ɾ��������ʹ��
void DeleteUCharArr(void* ptr)
{
    delete[] static_cast<unsigned char*>(ptr);
}

// ---------------- Image::Color ----------------
// ���캯��һ��ʹ�ó�ʼ���б�������ɫ��Ԫ����������ͨ�����ͣ�1~4��
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

// ���캯���������ڴӴ��ͼ�����ݵ��ֽ������а�ָ��ͨ�����Ϳ�����ɫֵ
Image::Color::Color(const unsigned char* arr, ChannelType channel_type)
{
    channel_type_ = channel_type;
    std::memcpy(value_, arr, static_cast<size_t>(channel_type_));
}

int Image::Color::value(int index) const
{
    if (index < 0 || index > (static_cast<int>(channel_type_) - 1))
        throw std::out_of_range("ȡ��ɫֵ������С����򳬹���ǰ�����ͨ����Ŀ��");
    return static_cast<int>(value_[index]);
}

void Image::Color::set_value(std::initializer_list<int> init_list)
{
    if (init_list.size() != static_cast<decltype(init_list.size())>(channel_type_))
        throw std::logic_error("������ɫֵʱ��ʼ��ֵ�����ϵ�ǰ�����ͨ����Ŀ��");
    int index = 0;
    for (const auto& element : init_list)
        value_[index++] = static_cast<unsigned char>(element);
}

// ---------------- Image ----------------
// ����հ�ͼ�񣺰������ֱ�����ͨ�����ͷ��䲢��������
Image::Image(int width, int height, ChannelType channel_type)
    : width_(width), height_(height), channel_type_(channel_type),
      data_(nullptr, DeleteUCharArr), flag_index_at_bottom_left_(true)
{
    assert(width_ > 0 && height_ > 0);
    data_.reset(new unsigned char[static_cast<size_t>(width_) * static_cast<size_t>(height_) * static_cast<size_t>(channel_type_)]());
}

// ���ļ�����ͼ���Զ����ͨ����
Image::Image(const char* path_to_file)
    : data_(nullptr, stbi_image_free), flag_index_at_bottom_left_(true)
{
    int num_channels;
    data_.reset(stbi_load(path_to_file, &width_, &height_, &num_channels, 0));
    if (!data_)
        throw std::runtime_error(stbi_failure_reason());
    channel_type_ = static_cast<ChannelType>(num_channels);
}

// ��ͼ��д���ļ���֧�� png/tga/jpg(jpeg)/bmp����׺��Сд
bool Image::WriteToFile(const char* path_to_file)
{
    assert(data_);
    std::string_view str(path_to_file);
    auto suffix_pos_minus_one = str.find_last_of('.');
    if (suffix_pos_minus_one == str.npos)
        return false;
    std::string_view suffix = str.substr(suffix_pos_minus_one + 1);

    if (suffix == std::string_view("png"))
        // stride_in_bytes = 0 ��ʾ��ͨ�����Զ������п�ȣ����������洢��
        return static_cast<bool>(stbi_write_png(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
    else if (suffix == std::string_view("tga"))
        return static_cast<bool>(stbi_write_tga(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
    else if (suffix == std::string_view("jpg") || suffix == std::string_view("jpeg"))
        // ���һ������Ϊ JPG ������0 ʹ�ÿ�Ĭ��ֵ��
        return static_cast<bool>(stbi_write_jpg(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
    else if (suffix == std::string_view("bmp"))
        return static_cast<bool>(stbi_write_bmp(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
    else
        return false;
}

// �� sRGB �ռ�����
void Image::ResizeInSrgbSpace(int new_width, int new_height)
{
    assert(data_);
    assert(new_width > 0 && new_height > 0);
    unsigned char* new_data = new unsigned char[static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channel_type_)];
    stbir_resize_uint8_srgb(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
        static_cast<stbir_pixel_layout>(channel_type_));
    // �� data_ ��ɾ�����л��Զ�������ɾ����
    data_.reset(new_data);
    data_.get_deleter() = DeleteUCharArr;
    width_ = new_width;
    height_ = new_height;
}

// �����Կռ�����
void Image::ResizeInLinearSpace(int new_width, int new_height)
{
    assert(data_);
    assert(new_width > 0 && new_height > 0);
    unsigned char* new_data = new unsigned char[static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channel_type_)];
    stbir_resize_uint8_linear(data_.get(), width_, height_, 0, new_data, new_width, new_height, 0,
        static_cast<stbir_pixel_layout>(channel_type_));
    data_.reset(new_data);
    data_.get_deleter() = DeleteUCharArr; // ɾ������Ҫ����������
    width_ = new_width;
    height_ = new_height;
}

// ��ȡ�������ص���ɫ������ԭ���־�����»����Ͽ�ʼ����
Image::Color Image::GetPixelColor(int x_index, int y_index) const
{
    assert(data_);
    if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
        throw std::out_of_range("�ڶ����ؽ�������ʱ������Χ��");

    int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
        : (y_index * width_ + x_index);
    unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
    return Color(pixel_ptr, channel_type_);
}

// ���õ������ص���ɫ����ɫ��ͨ����������ͼ��һ��
void Image::SetPixelColor(int x_index, int y_index, const Color& color)
{
    assert(data_);
    if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
        throw std::out_of_range("�ڶ����ؽ�������ʱ������Χ��");
    if (color.channel_type() != channel_type_)
        throw std::logic_error("�������ɫ��ͨ�������ͼ���ͨ�����ͬ��");

    int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
        : (y_index * width_ + x_index);
    unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
    std::memcpy(pixel_ptr, color.value(), static_cast<size_t>(channel_type_));
}

// ����ȫ�����ص���ɫ���˴���ʱ��ԭ����Ϊ���Ͻ��Լ�����������
void Image::SetAllPixelColor(const Color& color)
{
    assert(data_);
    if (color.channel_type() != channel_type_)
        throw std::logic_error("�������ɫ��ͨ�������ͼ���ͨ�����ͬ��");

    bool temp = flag_index_at_bottom_left_;
    flag_index_at_bottom_left_ = false;
    for (int x = 0; x < width_; ++x)
        for (int y = 0; y < height_; ++y)
            SetPixelColor(x, y, color);
    flag_index_at_bottom_left_ = temp;
}
