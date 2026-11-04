struct DISPLAY_MODE {
    int width;
    int height;
    int bpp;
    int vga;
    DISPLAY_MODE() {}
    bool CompareVideoModes(DISPLAY_MODE *mode);
};

extern "C" DISPLAY_MODE FullScreenMode;
DISPLAY_MODE FullScreenMode;
