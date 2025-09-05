//   Note: This file is encoded in GB2312!
//   ���ļ������˻��� stb_image ϵ�п�����ڻ���ͼ����� Image �༰���ڲ������� ChannelType �� Color����
// ֧�����ڴ����½��հ�ͼ����ߴ��ļ��ж�ȡͼ��֧�� 1~4 ͨ�����Ҷȡ��Ҷ�+Alpha��RGB��RGBA����ÿͨ�� 8 bit
// ��ͼ�����ݣ��ṩ�������صĶ�ȡ�����ã���ԭ�������»���������������ʽ������ͼ��ɫ��ͼ��д����png/tga/jpg/bmp
// ��ʽ���Լ��� sRGB �����Կռ��н��гߴ����ŵȽӿڡ�����ʵ�ַ����� image.cpp �С������ߣ�AniAdamX��
#pragma once
#ifndef MYIMAGE_IMAGE_H_
#define MYIMAGE_IMAGE_H_

#include <initializer_list>
#include <memory> // ��Ҫʹ�ã�unique_ptr��

//   Image�࣬��װ�� 1~4 ͨ�����Ҷ�/�Ҷ�+Alpha/RGB/RGBA���� 8 bit ÿͨ��������ͼ��֧��ͼ���д������
// ������ߴ����ŵȳ��ò�����
//   �˴���Ҫ��������÷����ӡ�
class Image {
public:
    // ǿö�����Ͷ����ͼ��ͨ���������kRgba�е�a��˼��kGrayAlpha��Alpha��ͬ����ָalphaͨ��
    enum class ChannelType {
        kGray = 1, kGrayAlpha, kRgb, kRgba
    };

    //   Color�࣬���������������ص���ɫֵ����֧����ChannelType���Ͷ�Ӧ��ͨ�������ÿͨ����֧��8����λ��
    // ����ֵֻ�ܴ�0��255����
    //   �˴���Ҫ��������÷����ӡ�
    class Color {
    public:
        // ���캯��һ��ʹ�� int ���ͳ�ʼ���б���Ϊ���ղ�����Ԫ�ظ�������ͨ������ 1~4��ֵ��Χ 0~255��
        Color(std::initializer_list<int> init_list);
        // ���캯�������� unsigned char ������ͨ�����͹��죬������ÿ��Ԫ�������Ű˱��ص�ͨ������
        Color(const unsigned char* arr, ChannelType channel_type);
        // ��ʽҪ���ƹ��캯������ֵ�������������������ʹ��Ĭ�Ϻϳɵİ汾
        Color(const Color&) = default;
        Color&  operator=(const Color&) = default;
        ~Color() = default;

        // ��ȡ����ĵ�ǰ Color �����ͨ�����
        ChannelType channel_type() const
        { return channel_type_; }
        void set_channel_type(ChannelType channel_type)
        { channel_type_ = channel_type; }
        // ��ȡ�������ɫֵ��value() �����ڲ��ֽ�������׵�ַ��ÿ�ֽڴ���һ��ͨ����ע����Ч�����뵱ǰ�����ͨ
        // ������Ӧ��value(i) ���ʵ� i ͨ��
        const unsigned char* value() const
        { return value_; }
        int value(int index) const;
        // ������ɫֵ����ʼ���б�������ڵ�ǰͨ����
        void set_value(std::initializer_list<int> init_list);

    private:
        // ָʾ��ǰColor�����ͨ�����
        ChannelType channel_type_;
        // ���ֽ��������飬��Ҫ����channel_type_�Ĳ�ͬ��ȷ����ǰ����Ԫ������Ч��
        unsigned char value_[4];
    };

    // ���캯��һ������һ��ָ���ֱ��ʺ�ͨ�����͵Ŀհ�ͼ����ֵΪȫ�㣩
    // ������width/height �������������ظ�����channel_type ͨ������
    Image(int width, int height, ChannelType channel_type);
    // ���캯���������ļ��е���ͼ���Զ�ȷ��ͨ������
    // ˵�����ڲ��ڵ���ʧ��ʱ�׳� std::runtime_error ��Я��ʧ��ԭ��
    Image(const char* path_to_file);

    // ��ϣ��ʹ�ÿ������캯���Ϳ�����ֵ�����
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    // ʹ��Ĭ�ϵ���������
    ~Image() = default;

    // ��ͼ������д���ļ����ɹ����� true
    // ֧�ָ�ʽ��png��tga��jpg/jpeg��bmp����׺��Сд��
    bool WriteToFile(const char* path_to_file);

    // ͼ�����ţ�sRGB �ռ䣩
    void ResizeInSrgbSpace(int new_width, int new_height);
    // ͼ�����ţ����Կռ䣩
    void ResizeInLinearSpace(int new_width, int new_height);

    // ����ȡֵ����
    int width() const
    { return width_; }
    int height() const
    { return height_; }
    ChannelType channel_type() const
    { return channel_type_; }
    // ��ȡ�������������ʱԭ��λ�ñ�־����true�����½ǣ�false�����Ͻǣ�
    bool flag_index_at_bottom_left() const
    { return flag_index_at_bottom_left_; }
    void set_flag_index_at_bottom_left(bool flag_index_at_bottom_left)
    { flag_index_at_bottom_left_ = flag_index_at_bottom_left; }
    // ��ȡͼ�񵥸����ص���ɫ
    Color GetPixelColor(int x_index, int y_index) const;
    // ����ͼ�񵥸����ص���ɫ
    void SetPixelColor(int x_index, int y_index, const Color& color);
    // ����ȫ�����ص���ɫ
    void SetAllPixelColor(const Color& color);

private:
    // ͼ��������ظ���
    int width_;
    // ͼ���������ظ���
    int height_;
    // ͼ��ͨ�����
    ChannelType channel_type_;
    // ָ��ͼ�����������ָ��
    std::unique_ptr<unsigned char, void(*)(void*)> data_;
    // ָʾ�ڶ���������ʱͼ��ԭ���Ƿ������½ǵı�־��Ĭ����true��false��ζ��ͼ��ԭ�������Ͻ�
    bool flag_index_at_bottom_left_;
};

#endif // MYIMAGE_IMAGE_H_
