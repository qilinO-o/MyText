#include "sub_editor.h"
#include <windows.h>
using namespace std;

class text_display{
public:
    HANDLE hOutput;//main screen
    CONSOLE_SCREEN_BUFFER_INFO bInfo;//main screen
    COORD screen_buffer_size;
    PSMALL_RECT window_size;
    struct cursor_location{
        int row;
        int col;
    };
    struct cursor_location cursor_loc;

    int display_top;//relative to the screen
    int display_line;
    int display_bottom;

    sub_editor::location file_top;//relative to the file(in the buffer contents)
    sub_editor::location file_line;
    sub_editor::location file_bottom;
    string nowmode;
    string nowcommand;
    string nowfile;

    bool window_Init();
    bool window_Fini();
    void clear_window();

    void display_content(sub_editor::storagebyline* content);
    void display_editor_info();
    void redisplaybyline(sub_editor::location &p);
    void redisplaydown(sub_editor::location &p,int count);

    bool cursor_Set(sub_editor::location &p);
	//bool cursor_Move_Col(int count);
    //bool cursor_Move_Row(int count);
	//cursor_location cursor_Get();
	//int cursor_Get_Line();
	cursor_location window_Start();
	cursor_location window_End();
};

bool text_display::window_Init(){
    HWND hwnd = GetForegroundWindow();
    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    LONG l_WinStyle = GetWindowLong(hwnd,GWL_STYLE);
    SetWindowLong(hwnd,GWL_STYLE,(l_WinStyle | WS_POPUP | WS_MAXIMIZE | WS_CAPTION |WS_THICKFRAME) & ~WS_VSCROLL & ~WS_THICKFRAME & ~WS_BORDER);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, cx, cy, 0);
    
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD fdwMode=ENABLE_WINDOW_INPUT | ENABLE_LINE_INPUT;
    PCONSOLE_CURSOR_INFO pConsoleCursorInfo=new CONSOLE_CURSOR_INFO;
    //DWORD dwFlags=CONSOLE_FULLSCREEN_MODE;
    //PCOORD pNewScreenBufferDimensions;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hInput, fdwMode);
    //SetConsoleDisplayMode(hOutput,dwFlags,pNewScreenBufferDimensions);
    screen_buffer_size.X=100;
    screen_buffer_size.Y=50;
    SetConsoleScreenBufferSize(hOutput,screen_buffer_size);
    window_size=new SMALL_RECT;
    window_size->Left=window_size->Top=0;
    window_size->Right=100;
    window_size->Bottom=50;
    SetConsoleWindowInfo(hOutput,FALSE,window_size);
    GetConsoleScreenBufferInfo(hOutput, &bInfo);
    GetConsoleCursorInfo(hOutput,pConsoleCursorInfo);
    pConsoleCursorInfo->bVisible=true;
    pConsoleCursorInfo->dwSize=10;
    SetConsoleCursorInfo(hOutput,pConsoleCursorInfo);
    cursor_loc={0,0};
    nowmode=nowcommand=nowfile="";
    return true;
}

bool text_display::window_Fini(){
    CloseHandle(hOutput);
    return true;
}

void text_display::clear_window(){
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;
    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hOutput, &bInfo)){
        return;
    }
    // Scroll the rectangle of the entire buffer.
    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = bInfo.dwSize.X;
    scrollRect.Bottom = bInfo.dwSize.Y;
    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - bInfo.dwSize.Y);
    // Fill with empty spaces with the buffer's default text attribute.
    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = bInfo.wAttributes;
    // Do the scroll
    ScrollConsoleScreenBuffer(hOutput, &scrollRect, NULL, scrollTarget, &fill);
    // Move the cursor to the top left corner too.
    bInfo.dwCursorPosition.X = 0;
    bInfo.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hOutput, bInfo.dwCursorPosition);
}

void text_display::display_content(sub_editor::storagebyline* content){
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    sub_editor::storagebyline* p=content;
    const char* el="\n";
    display_top=0;
    file_top.row=1;
    file_top.col=0;
    file_top.curline=p;
    int i=1;
    while(p!=NULL && i<window_size->Bottom-1){
        ++i;
        WriteConsoleA(hOutput,p->thisline->data,p->thisline->used,NULL,NULL);
        WriteConsoleA(hOutput,el,1,NULL,NULL);
        file_bottom.curline=p;
        p=p->next;
    }
    file_bottom.row=i-1;
    file_bottom.col=0;
    display_bottom=display_top+i;
    bInfo.dwCursorPosition.X = 0;
    bInfo.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hOutput, bInfo.dwCursorPosition);
}

void text_display::display_editor_info(){
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CHAR_INFO *editor_info;
    short int len=(nowmode.size()+nowcommand.size()+nowfile.size()+8);
    editor_info=(CHAR_INFO*)malloc(sizeof(CHAR_INFO)*window_size->Right);
    WORD info_attri;
    info_attri=BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
    int i;
    for(i=0;i<len;){
        for(int j=0;j<nowmode.size();++j,++i){
            (editor_info+i)->Char.AsciiChar=nowmode[j];
            (editor_info+i)->Attributes=info_attri;
        } 
        for(int j=0;j<4;++j,++i){
            (editor_info+i)->Char.AsciiChar=' ';
            (editor_info+i)->Attributes=info_attri;
        } 
        for(int j=0;j<nowcommand.size();++j,++i){
            (editor_info+i)->Char.AsciiChar=nowcommand[j];
            (editor_info+i)->Attributes=info_attri;
        } 
        for(int j=0;j<4;++j,++i){
            (editor_info+i)->Char.AsciiChar=' ';
            (editor_info+i)->Attributes=info_attri;
        } 
        for(int j=0;j<nowfile.size();++j,++i){
            (editor_info+i)->Char.AsciiChar=nowfile[j];
            (editor_info+i)->Attributes=info_attri;
        } 
    }
    for(;i<window_size->Right;++i){
        (editor_info+i)->Char.AsciiChar=' ';
        (editor_info+i)->Attributes=info_attri;
    }
    COORD dwBufferSize={window_size->Right,1};
    COORD dwBufferCoord={0,0};
    SMALL_RECT lpWriteRegion;
    lpWriteRegion.Bottom=window_size->Bottom;
    lpWriteRegion.Left=0;
    lpWriteRegion.Right=window_size->Right;
    lpWriteRegion.Top=window_size->Bottom;
    WriteConsoleOutput(hOutput,editor_info,dwBufferSize,dwBufferCoord,&lpWriteRegion);
}

bool text_display::cursor_Set(sub_editor::location &p){
    bInfo.dwCursorPosition.X = p.col;//col
    bInfo.dwCursorPosition.Y = p.row-1;//row
    cursor_loc={p.row-1,p.col};
    SetConsoleCursorPosition(hOutput, bInfo.dwCursorPosition);
    return true;
}

void text_display::redisplaybyline(sub_editor::location &p){
    display_line=p.row-file_top.row;
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CHAR_INFO *editor_info;
    short int len=window_size->Right;
    editor_info=(CHAR_INFO*)malloc(sizeof(CHAR_INFO)*len);
    WORD line_attri;
    line_attri=FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    for(int i=0;i<len;++i){
        if(i<p.curline->thisline->used) (editor_info+i)->Char.AsciiChar=p.curline->thisline->data[i];
        else (editor_info+i)->Char.AsciiChar=' ';
        (editor_info+i)->Attributes=line_attri;
    }
    COORD dwBufferSize={len,1};
    COORD dwBufferCoord={0,0};
    SMALL_RECT lpWriteRegion;
    lpWriteRegion.Bottom=display_line;
    lpWriteRegion.Left=0;
    lpWriteRegion.Right=len;
    lpWriteRegion.Top=display_line;
    WriteConsoleOutput(hOutput,editor_info,dwBufferSize,dwBufferCoord,&lpWriteRegion);
}

void text_display::redisplaydown(sub_editor::location &p,int count){
    sub_editor::location tp=p;
    if(count==0) return;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;
    // Scroll the rectangle of the entire buffer.
    scrollRect.Left = 0;
    scrollRect.Right = window_size->Right;
    scrollRect.Bottom = (SHORT)(display_bottom);
    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    scrollTarget.X = 0;
    if(count<0){
        scrollRect.Top = (SHORT)(p.row-count);
        scrollTarget.Y = (SHORT)(p.row);
    }
    else{
        scrollRect.Top = (SHORT)(p.row);
        scrollTarget.Y = (SHORT)(p.row+count);
    }
    // Fill with empty spaces with the buffer's default text attribute.
    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = bInfo.wAttributes;
    // Do the scroll
    ScrollConsoleScreenBuffer(hOutput, &scrollRect, NULL, scrollTarget, &fill);

    display_bottom+=count;
}