# MyImage - 基于STB库的C++图像处理库

这是一个使用 C++ （C++17）开发的轻量级图像处理库，基于著名的 C/C++ 第三方单文件库 [stb](https://github.com/nothings/stb) 中的图像编解码及缩放部分进行封装构建而成。本项目实现了完整的空白图像创建、图像读写、像素操作、尺寸缩放等功能，支持 1~4 通道（灰度、灰度 + Alpha 、RGB、RGBA）且每通道 8 bit 位深的图像数据。

# 使用到的C++特性及亮点

- 使用了现代C++语法，如 auto 关键字进行自动类型推断以简化代码，如用static_cast在编译期时实现了对部分运算过程中的安全的显式类型转换
- 使用了基于RAII资源管理思想的智能指针 std::unique_ptr ，配合自定义删除器自动管理图像内存
- 使用 std::initializer_list 以支持初始化列表语法
- 使用 std::string_view 来处理文件路径和扩展名，避免了不必要的字符串拷贝，实现性能提升
- 使用了嵌套类的设计，在 Image 类中定义enum class ChannelType 类和 Color 类，封装通道类别和像素颜色的逻辑
- 声明与实现分离，提高了编译效率
- 使用CMakeLists进行配置，支持CMake多平台构建
