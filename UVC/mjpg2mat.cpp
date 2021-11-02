#include "capture.h"




int main(int argc, char *argv[])
{
    char *device = "/dev/video2";
    uint32_t width = 1920;  //argc > 2 ? atoi(argv[2]) : 352;
    uint32_t height = 1080; //argc > 3 ? atoi(argv[3]) : 288;
    char *output = "result.jpg";

    camera_t *camera = camera_open(device);
    if (!camera)
    {
        fprintf(stderr, "[%s] %s\n", device, strerror(errno));
        return EXIT_FAILURE;
    }
    camera_format_t config = {1196444237, width, height, {1, 30}};
    if (!camera_config_set(camera, &config))
        return -1;
    if (!camera_config_get(camera, &config))
        return -1;
    char name[51];
    camera_format_name(config.format, name);
    printf("%s ha %d ha\n", name, strcmp(name, "MJPG"));
    if (strcmp(name, "YUYV") != 0 || strcmp(name, "MJPG") != 0)
    {
        fprintf(stderr, "camera format [%s] is not supported\n", name);
        // return -1;
    }

    if (!camera_start(camera))
        return -1;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    /* skip 5 frames for booting a cam */
    for (int i = 0; i < 5; i++)
    {
        camera_frame(camera, timeout);
    }
    camera_frame(camera, timeout);

    FILE *out = fopen(output, "w");
    if (strcmp(name, "YUYV") == 0)
    {
        unsigned char *rgb =
            yuyv2rgb(camera->head.start, camera->width, camera->height);
        // jpeg(out, rgb, camera->width, camera->height, 100);
        free(rgb);
    }
    // else if (strcmp(name, "MJPG"))
    {
        fwrite(camera->head.start, camera->head.length, 1, out);
        clock_t startTime, endTime;
        startTime = clock(); //计时开始
        std::vector<uchar> data;
        for (size_t i = 0; i < camera->head.length; i++)
        {
            data.push_back(camera->head.start[i]);
        }
        endTime = clock(); //计时结束
        std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
        startTime = clock(); //计时开始

        cv::Mat dst = cv::imdecode(data, cv::IMREAD_COLOR);
        endTime = clock(); //计时结束
        std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;

        startTime = clock(); //计时开始
        // load_jpeg(camera->head.start, camera->head.length);
        endTime = clock(); //计时结束
        std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;

        startTime = clock(); //计时开始
        cv::imwrite("haha.png", dst);
    }
    fclose(out);

    camera_stop(camera);
    camera_close(camera);
    return 0;
error:
    camera_close(camera);
    return EXIT_FAILURE;
}
