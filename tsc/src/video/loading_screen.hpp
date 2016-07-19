#ifndef TSC_LOADING_SCREEN_HPP
#define TSC_LOADING_SCREEN_HPP

namespace TSC {

    // initialize loading screen
    void Loading_Screen_Init(void);
    // set the loading screen info string and draw it
    // only call this after Loading_Screen_Init()
    void Loading_Screen_Draw_Text(const std::string& str_info = "Loading");
    // set the progress of the loading screen progres bar (range 0.0-1.0).
    // only call this after Loading_Screen_Init()
    void Loading_Screen_Set_Progress(float progress);
    // draw the loading screen
    // only call this after Loading_Screen_Init()
    void Loading_Screen_Draw(void);
    // exit loading screen
    // only call this after Loading_Screen_Init()
    void Loading_Screen_Exit(void);

}

#endif
