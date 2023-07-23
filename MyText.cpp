#include "text_display.h"
#include <conio.h>

using namespace std;

#define NORMAL_MODE 0
#define INSERT_MODE 1

void Read_Command(sub_editor &Sub_Editor,text_display &Text_Display){
    int mode=NORMAL_MODE;
    Text_Display.nowmode="normal mode";
    Text_Display.display_editor_info();
    string curcommand;
    while(1){
        if(mode==NORMAL_MODE){
            char c=getch();
            switch(c){
                case ':':
                    {
                        string sub_command="";
                        string whole_cmd=":";
                        Text_Display.nowcommand=whole_cmd;
                        Text_Display.display_editor_info();
                        char cc;
                        while(1){
                            cc=getch();
                            if(cc!=0x20 && cc!=0xd){
                                sub_command+=cc;
                                whole_cmd+=cc;
                                Text_Display.nowcommand=whole_cmd;
                                Text_Display.display_editor_info();
                            } 
                            else break;
                        } 
                        if(sub_command[0]=='q'){
                            Text_Display.clear_window();
                            return;
                        }
                        else if(sub_command[0]=='o'){
                            string filename="";
                            whole_cmd+=' ';
                            while(1){
                                cc=getch();
                                if(cc!=0xd){
                                    filename+=cc;
                                    whole_cmd+=cc;
                                    Text_Display.nowcommand=whole_cmd;
                                    Text_Display.display_editor_info();
                                } 
                                else break;
                            } 
                            Sub_Editor.Set_File_Name(filename);
                            sub_editor::status fs=Sub_Editor.Buffer_Read(filename);
                            if(fs.state=="Fail Open File"){
                                Text_Display.nowfile="Error file";
                                Text_Display.display_editor_info();
                                continue;
                            }
                            Text_Display.nowfile=filename;
                            Text_Display.display_editor_info();
                            Sub_Editor.Point_Set({1,0});
                            Text_Display.display_content(Sub_Editor.all_buffer.current_buffer->contents);
                        }
                        else if(sub_command[0]=='w'){
                            string filename="";
                            whole_cmd+=' ';
                            while(1){
                                cc=getch();
                                if(cc!=0xd){
                                    filename+=cc;
                                    whole_cmd+=cc;
                                    Text_Display.nowcommand=whole_cmd;
                                    Text_Display.display_editor_info();
                                }
                                else break;
                            }
                            Sub_Editor.Buffer_Write(filename);
                        }
                        else continue;
                    }
                    break;
                case 'h'://left
                    Sub_Editor.point_Move_Left();
                    Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    break;
                case 'j'://down
                    Sub_Editor.point_Move_Down();
                    Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    break;
                case 'k'://up
                    Sub_Editor.point_Move_Up();
                    Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    break;
                case 'l'://right
                    Sub_Editor.point_Move_Right();
                    Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    break;
                case '/'://search text
                    {
                        string sub_command="";
                        Text_Display.nowcommand="/"+sub_command;
                        Text_Display.display_editor_info();
                        char cc;
                        while(1){
                            cc=getch();
                            if(cc!=0xd){
                                sub_command+=cc;
                                Text_Display.nowcommand="/"+sub_command;
                                Text_Display.display_editor_info();
                            } 
                            else break;
                        }
                        Sub_Editor.search_behind(sub_command);
                        Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    }
                    break;
                case 'i'://get into insert mode
                    mode=INSERT_MODE;
                    Text_Display.nowmode="insert mode";
                    Text_Display.display_editor_info();
                    break;
                case 'u'://undo
                    {
                        int cmd=Sub_Editor.undo();
                        if(cmd==1){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        }
                        else if(cmd==2){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        }
                        else if(cmd==3){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                            Text_Display.redisplaydown(Sub_Editor.all_buffer.current_buffer->point,-1);
                        }
                        else if(cmd==4){
                            sub_editor::location temp={Sub_Editor.all_buffer.current_buffer->point.row-1,
                                                    0,Sub_Editor.all_buffer.current_buffer->point.curline->pre};
                            Text_Display.redisplaydown(temp,1);

                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                            temp={Sub_Editor.all_buffer.current_buffer->point.row+1,
                                                    0,Sub_Editor.all_buffer.current_buffer->point.curline->next};
                            Text_Display.redisplaybyline(temp);
                        }
                        Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    }
                    break;
                case 'r'://redo
                    {
                        int cmd=Sub_Editor.redo();
                        if(cmd==1){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        }
                        else if(cmd==2){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        }
                        else if(cmd==3){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                            Text_Display.redisplaydown(Sub_Editor.all_buffer.current_buffer->point,-1);
                        }
                        else if(cmd==4){
                            sub_editor::location temp={Sub_Editor.all_buffer.current_buffer->point.row-1,
                                                    0,Sub_Editor.all_buffer.current_buffer->point.curline->pre};
                            Text_Display.redisplaydown(temp,1);

                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                            temp={Sub_Editor.all_buffer.current_buffer->point.row+1,
                                                    0,Sub_Editor.all_buffer.current_buffer->point.curline->next};
                            Text_Display.redisplaybyline(temp);
                        }
                        Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    }
                    break;
                case 'x'://delete char
                    {
                        if(!Sub_Editor.Delete(1)){
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        }
                        else{
                            Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                            Text_Display.redisplaydown(Sub_Editor.all_buffer.current_buffer->point,-1);
                            Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                        } 
                    }
                    break;
                default:
                    break;
            }
        }
        else if(mode==INSERT_MODE){
            char c=getch();
            char dir;
            switch(c){
                case 0x1B://esc
                    mode=NORMAL_MODE;
                    Text_Display.nowmode="normal mode";
                    Text_Display.display_editor_info();
                    break;
                case -32://perhaps will be 0 or 224
                    dir=getch();
                    switch(dir){
                        case 0x4B://left
                            Sub_Editor.point_Move_Left();
                            Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                            break;
                        case 0x50://down
                            Sub_Editor.point_Move_Down();
                            Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                            break;
                        case 0x48://up
                            Sub_Editor.point_Move_Up();
                            Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                            break;
                        case 0x4D://right
                            Sub_Editor.point_Move_Right();
                            Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                            break;
                    }
                    break;
                default:
                    if(c==0xd){
                        sub_editor::location temp={Sub_Editor.all_buffer.current_buffer->point.row,
                                                0,Sub_Editor.all_buffer.current_buffer->point.curline};
                        Text_Display.redisplaydown(temp,1);
                        
                        Sub_Editor.New_line(Sub_Editor.all_buffer.current_buffer->point.curline,
                                            Sub_Editor.all_buffer.current_buffer->point.col);
                        
                        Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        temp={Sub_Editor.all_buffer.current_buffer->point.row+1,
                                                0,Sub_Editor.all_buffer.current_buffer->point.curline->next};
                        Text_Display.redisplaybyline(temp);
                        Sub_Editor.point_Move_Right();
                        Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    }
                    else if(c==0x09 || (c>=0x20 && c<=0x7E)){
                        Sub_Editor.Insert_Char(c);
                        Text_Display.redisplaybyline(Sub_Editor.all_buffer.current_buffer->point);
                        Text_Display.cursor_Set(Sub_Editor.all_buffer.current_buffer->point);
                    }
                    break;
            }
        }
    }
}

int main()
{
    sub_editor Sub_Editor;
    text_display Text_Display;
    Sub_Editor.all_buffer_Init();
    string defaultname="default";
    Sub_Editor.buffer_Create(defaultname);
    Text_Display.window_Init();
    Read_Command(Sub_Editor,Text_Display);
    return 0;
}
