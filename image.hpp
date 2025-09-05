// !!! GB2312 Encoded !!!
// ������stb_image������ͷ�ļ�������һ��Image�࣬���Զ�ͼ����ж�д�Ȳ���
// �����ߣ�AniAdamX��
#pragma once
#ifndef MYIMAGE_IMAGE_H_
#define MYIMAGE_IMAGE_H_

#include <cassert> // assert()����
#include <cstring> // memcpy()����

#include <initializer_list>
#include <stdexcept> // logic_error��out_of_range��runtime_error��
#include <memory> // unique_ptr��
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

// �˴�������Ĺ��ܸ������÷�����
// WriteToFile()������֧��PNG��TGA��JPG��JPEG����BMP���ָ�ʽ�Һ�׺����Сд
class Image {
public:
    // ǿö�����Ͷ����ͼ��ͨ���������kRgba�е�a��˼��kGrayAlpha��Alpha��ͬ����ָalphaͨ��
    enum class ChannelType {
        kGray = 1, kGrayAlpha, kRgb, kRgba
    };

    // �˴�������Ĺ��ܸ������÷�����
    // ��֧����ChannelType���Ͷ�Ӧ��ͨ�������ÿͨ����֧��8����λ���ֵֻ�ܴ�0��255��
    class Color {
    public:
        // ���캯��һ��ʹ��int���ͳ�ʼ���б���Ϊ����
        Color(initializer_list<int> init_list)
        {
            // �����жϳ�ʼ���б���ֵ�ĸ�������Ҫ���ҳ�ʼ��ֵ����Ч��Χ��
            assert(init_list.size() != 0 && init_list.size() < 5);
            for (const auto& element : init_list)
                assert(element >= 0 && element <= 255);

            channel_type_ = static_cast<ChannelType>(init_list.size());
            int index = 0;
            for (const auto& element : init_list)
                value_[index++] = element;
        }
        // ���캯������ʹ��unsigned char���飨ָ�룩��ͨ��������Ϊ����
        Color(const unsigned char* arr, ChannelType channel_type)
        {
            channel_type_ = channel_type;
            memcpy(value_, arr, static_cast<size_t>(channel_type_));
        }
        // ��ʽҪ���ƹ��캯������ֵ�������������������ʹ��Ĭ�Ϻϳɵİ汾
        Color(const Color&) = default;
        Color&  operator=(const Color&) = default;
        ~Color() = default;

        // ��ȡ����ĵ�ǰColor�����ͨ�����
        ChannelType channel_type() const
        { return channel_type_; }
        void set_channel_type(ChannelType channel_type)
        { channel_type_ = channel_type; }
        // ��ȡ�������ɫֵ
        const unsigned char* value() const
        { return value_; }
        int value(int index) const
        {
            if (index < 0 || index > (static_cast<int>(channel_type_) - 1))
                throw out_of_range("ȡ��ɫֵ������С����򳬹���ǰ�����ͨ����Ŀ��");
            return static_cast<int>(value_[index]);
        }
        void set_value(initializer_list<int> init_list)
        {
            if (init_list.size() != static_cast<decltype(init_list.size())>(channel_type_))
                throw logic_error("������ɫֵʱ��ʼ��ֵ�����ϵ�ǰ�����ͨ����Ŀ��");
            int index = 0;
            for (const auto& element : init_list)
                value_[index++] = element;
        }

    private:
        // ָʾ��ǰColor�����ͨ�����
        ChannelType channel_type_;
        // ���ֽ��������飬��Ҫ����channel_type_�Ĳ�ͬ��ȷ����ǰ����Ԫ������Ч��
        unsigned char value_[4];
    };

    // ���캯��������һ��ָ���ֱ��ʺ�ͨ�����͵Ŀհ�ͼ����ֵΪȫ�㣩
    Image(int width, int height, ChannelType channel_type) : width_(width), height_(height), channel_type_(channel_type),
        data_(nullptr, DeleteUCharArr), flag_index_at_bottom_left_(true)
    {
        // ���ù��캯��ʱ��Ӧ�ô�����Чֵ��������û�������Ӧ���ڵ��ù��캯��ǰ���м�鴦��
        assert(width_ > 0 && height_ > 0);
        // ����������bad_alloc�쳣
        data_.reset(new unsigned char[width_ * height_ * static_cast<int>(channel_type_)]());
    }

    // ���캯�������ļ��е���ͼ��
    Image(const char* path_to_file) : data_(nullptr, stbi_image_free), flag_index_at_bottom_left_(true)
    {
        int num_channels;
        data_.reset(stbi_load(path_to_file, &width_, &height_, &num_channels, 0));

        // �������ͼ��ʧ�ܣ�������������͵��׳��쳣
        if (!data_)
            throw runtime_error(stbi_failure_reason());

        channel_type_ = static_cast<ChannelType>(num_channels);
    }

    // ��ϣ��ʹ�ÿ������캯���Ϳ�����ֵ�����
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    // ʹ��Ĭ�ϵ���������
    ~Image() = default;

    // ��ͼ������д���ļ����ɹ�����true
    bool WriteToFile(const char* path_to_file)
    {
        assert(data_);
        // ��ȡ�ļ���׺��
        string_view str(path_to_file);
        auto suffix_pos_minus_one = str.find_last_of('.');
        if (suffix_pos_minus_one == str.npos)
            return false;
        string_view suffix = str.substr(suffix_pos_minus_one + 1);
        // ���ݺ�׺������stb_image_write��ĺ���
        if (suffix == string_view("png"))
            // ����ͼ�������������洢�ļ����ض����ڴ���룬��˴˴�����ʱȡ����stride_in_bytes = 0ʹ���Զ�����
            return static_cast<bool>(stbi_write_png(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get(), 0));
        else if (suffix == string_view("tga"))
            return static_cast<bool>(stbi_write_tga(path_to_file, width_, height_, static_cast<int>(channel_type_), data_.get()));
        else if (suffix == string_view("jpg") || suffix == string_view("jpeg"))
            // ���һ������������JPG��ʽ��ѹ��������0����ʾʹ�ÿ��Ĭ��ֵ
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
            static_cast<stbir_pixel_layout>(channel_type_)); // ����0��ʾ���ִ���
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
            static_cast<stbir_pixel_layout>(channel_type_)); // ����0��ʾ���ִ���
        data_.reset(new_data);
        data_.get_deleter() = DeleteUCharArr;
        width_ = new_width;
        height_ = new_height;
    }

    // ����ȡֵ����
    int width() const
    { return width_; }
    int height() const
    { return height_;}
    ChannelType channel_type() const
    { return channel_type_; }
    // ��ȡ�������������ʱԭ��λ�ñ�־��
    bool flag_index_at_bottom_left()  const
    { return flag_index_at_bottom_left_; }
    void set_flag_index_at_bottom_left(bool flag_index_at_bottom_left)
    { flag_index_at_bottom_left_ = flag_index_at_bottom_left; }
    // ��ȡͼ�񵥸����ص���ɫ
    Color GetPixelColor(int x_index, int y_index) const
    {
        assert(data_);
        if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
            throw out_of_range("�ڶ����ؽ�������ʱ������Χ��");
        
        int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + width_)
            : (y_index * width_ + x_index);
        unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
        return Color(pixel_ptr, channel_type_);
    }
    // ����ͼ�񵥸����ص���ɫ
    void SetPixelColor(int x_index, int y_index, const Color& color)
    {
        assert(data_);
        if (x_index < 0 || y_index < 0 || x_index >= width_ || y_index >= height_)
            throw out_of_range("�ڶ����ؽ�������ʱ������Χ��");
        if (color.channel_type() != channel_type_)
            throw logic_error("�������ɫ��ͨ�������ͼ���ͨ�����ͬ��");

        int pixel_pos = flag_index_at_bottom_left_ ? ((height_ - 1 - y_index) * width_ + x_index)
            : (y_index * width_ + x_index);
        unsigned char* pixel_ptr = data_.get() + pixel_pos * static_cast<int>(channel_type_);
        memcpy(pixel_ptr, color.value(), static_cast<size_t>(channel_type_));
    }
    // ����ȫ�����ص���ɫ
    void SetAllPixelColor(const Color& color)
    {
        assert(data_);
        bool temp = flag_index_at_bottom_left_;

        // ������������ԭ��λ�����Ͻ��Լ���������
        flag_index_at_bottom_left_ = false;
        for (int x = 0; x < width_; ++x)
            for (int y = 0; y < height_; ++y)
                SetPixelColor(x, y, color);
        flag_index_at_bottom_left_ = temp;
    }

private:
    // ͼ��������ظ���
    int width_;
    // ͼ���������ظ���
    int height_;
    // ͼ��ͨ�����
    ChannelType channel_type_;
    // ָ��ͼ�����������ָ�루ԭʼ����ͼ��ԭ���ǣ�
    unique_ptr<unsigned char, void(*)(void*)> data_;
    // ָʾ�ڶ���������ʱͼ��ԭ���Ƿ������½ǵı�־��Ĭ����true��false��ζ��ͼ��ԭ�������Ͻ�
    bool flag_index_at_bottom_left_;
};

#endif // MYIMAGE_IMAGE_H_
