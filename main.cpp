#include "image.hpp"

int main(int argc, char** argv)
{
    Image img(100, 100, Image::ChannelType::kRgba);
    const Image::Color black_rgba({0, 0, 0, 255});
    const Image::Color red_rgba({255, 0, 0, 255});

    img.SetAllPixelColor(black_rgba);
    img.SetPixelColor(30, 40, red_rgba);

    if (argc > 1)
        img.WriteToFile(argv[1]);
    else
        img.WriteToFile("output.png");

    return 0;
}
