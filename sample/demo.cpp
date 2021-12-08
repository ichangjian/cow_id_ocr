#include "cow_sdk.hpp"
#include <unistd.h>
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    COW *cow = nullptr;
    if (argc > 1)
    {
        std::cout << "corner path=<" << argv[1] << ">\n";
        std::cout << "vedio path=<" << argv[2] << ">\n";
        cow = new COW(argv[1]);
        cow->testVideo(argv[2]);
    }
    else
    {
        sleep(10);
        cow = new COW("/sdcard/Download/corner.yaml");
    }
    std::cout << cow->getVersion() << "\n";
    cow->init();
    while (true)
        getchar();
    delete cow;
}
