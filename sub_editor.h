#include <bits/stdc++.h>
using namespace std;

class sub_editor{
public:
    //basic storage structure
    int Round_Up_To_Block_Size(int x){
        int t=8;
        while(t<=x) t<<=1;
        return t;
    }
    struct line {
		int length;
		int used;
		char* data;	/* length characters */
	};
    line* inlineinsert(line* curline, int start,int len,char* insertcontent);
    line* inlinedelete(line* curline, int start,int len);
    struct storagebyline {
		struct storagebyline *next;
		struct storagebyline *pre;
		struct line *thisline;
		//struct marks *mark_lists;//marks on this line
	};
    //**************************************
    
    //marks that remember certain locations in a file
    struct location{
        int row,col;//row start from 1 and col start from 0;
        struct storagebyline *curline;
    };
    struct mark_name{
        string name;
        struct mark *current_mark;
    };
    struct mark{
        struct mark *next_mark;
		mark_name name;
		location pos;
		bool is_fixed;
    };
    //*********************************************

    struct status{
        string state;
    };
    //undo data struct
    struct one_undo{
        int cmd;
        char c;
        location loc;
        /*cmd
        1:insert a char
        2:delete a char
        3:make a new line
        4:delete a line
        */
    };
    //*********************************************

    //buffer that contains a file
    typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> microClock_type;
    struct buffer{ //double directed circular linklist
        struct buffer *pre_buffer;
        struct buffer *next_buffer;
		string buffer_name;
        
		location point;
		//int cur_line;
		int num_chars;
		int num_lines;

		struct mark *mark_list;

		struct storagebyline *contents;//start node of the content link

		string file_name;
		microClock_type file_time;
		bool is_modified;

        deque<one_undo> undo_stack;
        deque<one_undo> redo_stack;
        buffer(){
            pre_buffer=next_buffer=NULL;
            point={0,0,NULL};
            num_chars=num_lines=0;
            mark_list=NULL;
            contents=NULL;
        }
    };
    //*************************************
    
    //handle several files
    struct All_buffer{
        struct buffer *buffer_chain;//start node of the buffer link
        struct buffer *current_buffer;
        struct status all_buffer_status;
    }all_buffer;
    //************************************

    //functions of the class: sub_editor
    bool all_buffer_Init();
	bool all_buffer_Fini();
    
    buffer* buffer_Create(string buffer_name);
	bool buffer_Clear(string buffer_name);
	bool buffer_Delete(string buffer_name);
	bool buffer_Set_Current(string buffer_name);
	string buffer_Set_Next();
	bool buffer_Set_Name(string buffer_name);
	string buffer_Get_Name();

    status Point_Set(location loc);
	bool Point_Move_Col(int count);
    bool Point_Move_Row(int count);
    bool point_Move_Left();
    bool point_Move_Right();
    bool point_Move_Up();
    bool point_Move_Down();
	location Point_Get();
	int Point_Get_Line();
	location Buffer_Start();
	location Buffer_End();

    //char Get_Char();
	//void Get_String(char *string, int count); //for copy etc.
	int Get_Num_Chars();
	int Get_Num_Lines();

    void Get_File_Name(string file_name);
	status Set_File_Name(string file_name);
	status Buffer_Write(string file_name);
	status Buffer_Read(string file_name);
	//bool Is_File_Changed();
	void Set_Modified(bool is_modified);
	bool Get_Modified();

    //for undo
    void undo_iffull();

    //for insert and delete
    void Delete_line(storagebyline* linetodelete);
    void New_line(storagebyline* thisline,int split_pos);
    void Insert_Char(char c);
    bool Delete(int count);

    //for search
    bool search_behind(string str);
    //for undo
    int undo();
    int redo();
};

//functions of the all_buffer
bool sub_editor::all_buffer_Init(){
    string defaultname="Default buffer";
    all_buffer.current_buffer=buffer_Create(defaultname);
    all_buffer.buffer_chain=all_buffer.current_buffer; //set the start
    if(all_buffer.current_buffer==NULL){
        all_buffer.all_buffer_status.state="Fail Init";
        return false;
    } 
    else{
        all_buffer.all_buffer_status.state="Success Init";
        return true;
    } 
}
bool sub_editor::all_buffer_Fini(){
    if(all_buffer.all_buffer_status.state=="Fail Init") return false;
    while(all_buffer.current_buffer!=NULL){
        buffer* temp=all_buffer.current_buffer;
        all_buffer.current_buffer=all_buffer.current_buffer->next_buffer;
        free(temp);
    }
    all_buffer.current_buffer=all_buffer.buffer_chain=NULL;
    all_buffer.all_buffer_status.state="Fail Init";
    return true;
}
//****************************************

//functions of the buffer itself
sub_editor::buffer* sub_editor::buffer_Create(string buffer_name){
    buffer* newbuffer=new buffer;
    newbuffer->buffer_name=buffer_name;
    newbuffer->next_buffer=newbuffer->pre_buffer=NULL;
    newbuffer->num_chars=newbuffer->num_lines=0;
    newbuffer->point={0,0};
    newbuffer->contents=NULL;
    newbuffer->file_name="";
    newbuffer->is_modified=false;
    newbuffer->mark_list=NULL;
    return newbuffer;
}
bool sub_editor::buffer_Clear(string buffer_name){
    buffer* p=all_buffer.current_buffer;
    while(p->buffer_name!=buffer_name) p=p->next_buffer;
    while(p->contents!=NULL){
        if(p->contents->thisline!=NULL && p->contents->thisline->data!=NULL)
            free(p->contents->thisline->data);
        if(p->contents->thisline!=NULL) free(p->contents->thisline);
        storagebyline* temp=p->contents;
        p->contents=p->contents->next;
        free(temp);
    }
    p->point={0,0,NULL};
    p->file_name="";
    p->is_modified=false;
    p->num_chars=p->num_lines;
    while(p->mark_list!=NULL){
        mark* temp=p->mark_list;
        p->mark_list=p->mark_list->next_mark;
        free(temp);
    }
    return true;
}
bool sub_editor::buffer_Delete(string buffer_name){
    buffer* p=all_buffer.current_buffer;
    while(p->buffer_name!=buffer_name) p=p->next_buffer;
    if(p==all_buffer.current_buffer) all_buffer.current_buffer=all_buffer.current_buffer->next_buffer;
    if(p==all_buffer.buffer_chain) all_buffer.buffer_chain=all_buffer.buffer_chain->next_buffer;
    p->pre_buffer->next_buffer=p->next_buffer;
    p->next_buffer->pre_buffer=p->pre_buffer;
    buffer_Clear(p->buffer_name);
    free(p);
    return true;
}
bool sub_editor::buffer_Set_Current(string buffer_name){
    buffer* p=all_buffer.current_buffer;
    while(p->buffer_name!=buffer_name) p=p->next_buffer;
    all_buffer.current_buffer=p;
    return true;
}
string sub_editor::buffer_Set_Next(){
    all_buffer.current_buffer=all_buffer.current_buffer->next_buffer;
    return all_buffer.current_buffer->buffer_name;
}
bool sub_editor::buffer_Set_Name(string buffer_name){
    all_buffer.current_buffer->buffer_name=buffer_name;
    return true;
}
string sub_editor::buffer_Get_Name(){
    return all_buffer.current_buffer->buffer_name;
}
//******************************************

//functions of the storagebyline and line
sub_editor::line* sub_editor::inlineinsert(line* curline, int start,int len,char* insertcontent){
    int can_insert_length = min(curline->length - curline->used, len);
    memmove(&curline->data[start + can_insert_length], &curline->data[start],curline->used - start);
	memmove(&curline->data[start], insertcontent, can_insert_length);
	curline->used += can_insert_length;
	len -= can_insert_length;
	if (len <= 0) return curline;	//insert wholely finished

    start += can_insert_length;
	insertcontent += can_insert_length;
    line* newline;
    int newlen = Round_Up_To_Block_Size(curline->length + len);
    newline=(line*)malloc(sizeof(line));
    newline->data=(char*)malloc(sizeof(char)*newlen);

    // copy rest contents
    memmove(&newline->data[0], curline->data, start);
    memmove(&newline->data[start], insertcontent, len);
    memmove(&newline->data[start + len], curline->data+start,curline->length - start);

    newline->length = newlen;
    newline->used = curline->used + len;
    free(curline);
    return newline;
}
sub_editor::line* sub_editor::inlinedelete(line* curline, int start,int len){
    if(len==0) return curline;
    memmove(&curline->data[start], &curline->data[start + len],curline->used - (start + len));
	curline->used -= len;
	return curline;
}
//******************************************

//functions inside the buffer
sub_editor::status sub_editor::Point_Set(location loc){
    status pointstate;
    if(loc.row>all_buffer.current_buffer->num_lines) pointstate.state="Point Out Range";
    else{
        storagebyline* templine=all_buffer.current_buffer->contents;
        for(int i=1;i<loc.row;++i) templine=templine->next;
        if(loc.col>templine->thisline->used) pointstate.state="Point Out Range";
        else{
            all_buffer.current_buffer->point.curline=templine;
            all_buffer.current_buffer->point.col=loc.col;
            all_buffer.current_buffer->point.row=loc.row;
            pointstate.state="Point Set Succ";
        }
    }
    return pointstate;
}
bool sub_editor::Point_Move_Col(int count){
    all_buffer.current_buffer->point.col+=count;
    if(all_buffer.current_buffer->point.col<0) all_buffer.current_buffer->point.col=0;
    else if(all_buffer.current_buffer->point.col>all_buffer.current_buffer->point.curline->thisline->used)
        all_buffer.current_buffer->point.col=all_buffer.current_buffer->point.curline->thisline->used;
    return true;
}
bool sub_editor::Point_Move_Row(int count){
    bool ifexceed=false;
    all_buffer.current_buffer->point.row+=count;
    if(all_buffer.current_buffer->point.row<1) all_buffer.current_buffer->point.row=1;
    else if(all_buffer.current_buffer->point.row>all_buffer.current_buffer->num_lines)
        all_buffer.current_buffer->point.row=all_buffer.current_buffer->num_lines;
    if(count>=0){//move down
        while(count && all_buffer.current_buffer->point.curline->next!=NULL){
            --count;
            all_buffer.current_buffer->point.curline=all_buffer.current_buffer->point.curline->next;
        }
        if(count!=0) ifexceed=true;
    }
    else{//move up
        count=-1*count;
        while(count && all_buffer.current_buffer->point.curline->pre!=NULL){
            --count;
            all_buffer.current_buffer->point.curline=all_buffer.current_buffer->point.curline->pre;
        }
        if(count!=0) ifexceed=true;
    }
    return ifexceed;
}
sub_editor::location sub_editor::Point_Get(){
    return all_buffer.current_buffer->point;
}
int sub_editor::Point_Get_Line(){
    return all_buffer.current_buffer->point.row;
}
sub_editor::location sub_editor::Buffer_Start(){
    location loc;
    loc.col=0;
    loc.row=1;
    loc.curline=all_buffer.current_buffer->contents;
    return loc;
}
sub_editor::location sub_editor::Buffer_End(){
    location loc;
    storagebyline* templine=all_buffer.current_buffer->point.curline;
    while(templine->next!=NULL) templine=templine->next;
    loc.row=all_buffer.current_buffer->num_lines;
    loc.col=templine->thisline->used-1;
    loc.curline=templine;
    return loc;
}

bool sub_editor::point_Move_Left(){
    if(all_buffer.current_buffer->point.col==0){
        if(!Point_Move_Row(-1)){
            Point_Move_Col(all_buffer.current_buffer->point.curline->thisline->used);
        }
    }
    else{
        Point_Move_Col(-1);
    }
    return true;
}
bool sub_editor::point_Move_Right(){
    int count=1;
    if(all_buffer.current_buffer->point.col==all_buffer.current_buffer->point.curline->thisline->used){
        if(!Point_Move_Row(1)){
            Point_Move_Col(0-all_buffer.current_buffer->point.col);
        }
    }
    else{
        Point_Move_Col(1);
    }
    return true;
}
bool sub_editor::point_Move_Up(){
    Point_Move_Row(-1);
    Point_Move_Col(0);
    return true;
}
bool sub_editor::point_Move_Down(){
    Point_Move_Row(1);
    Point_Move_Col(0);
    return true;
}
//*******************************************

//functions dealing with the info of the buffer
int sub_editor::Get_Num_Chars(){
    return all_buffer.current_buffer->num_chars;
}
int sub_editor::Get_Num_Lines(){
    return all_buffer.current_buffer->num_lines;
}
//*******************************************

//functions dealing with the content of the buffer
void sub_editor::Get_File_Name(string file_name){
    file_name=all_buffer.current_buffer->file_name;
}
sub_editor::status sub_editor::Set_File_Name(string file_name){
    status ifcover;
    if(all_buffer.current_buffer->file_name!="") ifcover.state="Cover Old File";
    else ifcover.state="No Cover";
    all_buffer.current_buffer->file_name=file_name;
    return ifcover;
}
sub_editor::status sub_editor::Buffer_Write(string file_name){
    ofstream datafile;
    status writestate;
    datafile.open(file_name,ios::out | ios::trunc);
    if(!datafile.is_open()){
        writestate.state="Fail Open File";
        return writestate;
    }
    //out by line
    storagebyline *p=all_buffer.current_buffer->contents;
    while(p!=NULL){
        for(int i=0;i<p->thisline->used;++i) datafile<<p->thisline->data[i];
        datafile<<endl;
        p=p->next;
    }
    datafile.close();
    all_buffer.current_buffer->is_modified=false;
    all_buffer.current_buffer->file_time=chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
    writestate.state="Succ Write File";
    return writestate;
}
sub_editor::status sub_editor::Buffer_Read(string file_name){
    buffer_Clear(all_buffer.current_buffer->buffer_name);
    Set_File_Name(file_name);
    ifstream datafile;
    status readstate;
    //in by line
    datafile.open(all_buffer.current_buffer->file_name.data(),ios::in);
    if(!datafile.is_open()){
        readstate.state="Fail Open File";
        return readstate;
    }
    all_buffer.current_buffer->num_lines=0;
    all_buffer.current_buffer->num_chars=0;
    string templine;
    storagebyline* pre=NULL;
    while(getline(datafile,templine)){
        storagebyline* p=new storagebyline;
        if(pre!=NULL) pre->next=p;
        p->pre=pre;p->next=NULL;
        pre=p;
        p->thisline=new line;
        int len=Round_Up_To_Block_Size(templine.length());
        //new a line of txt
        p->thisline->data=(char*)malloc(sizeof(char)*len);
        strcpy(p->thisline->data,templine.data());
        p->thisline->length=len;
        p->thisline->used=templine.length();
        ++all_buffer.current_buffer->num_lines;
        all_buffer.current_buffer->num_chars+=templine.length();
        if(p->pre==NULL) all_buffer.current_buffer->contents=p;
    }
    if(all_buffer.current_buffer->contents==NULL){
        storagebyline* p=new storagebyline;
        p->thisline=new line;
        p->thisline->data=(char*)malloc(sizeof(char)*16);
        p->thisline->length=16;
        p->thisline->used=0;
        p->pre=p->next=NULL;
        ++all_buffer.current_buffer->num_lines;
        all_buffer.current_buffer->contents=p;
    }
    datafile.close();
    all_buffer.current_buffer->is_modified=false;
    all_buffer.current_buffer->file_time=chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
    readstate.state="Succ Read File";
    return readstate;
}
void sub_editor::Set_Modified(bool is_modified){
    all_buffer.current_buffer->is_modified=is_modified;
}
bool sub_editor::Get_Modified(){
    return all_buffer.current_buffer->is_modified;
}
//*******************************************

//functions manipulating the buffer for insert and delete
void sub_editor::Delete_line(storagebyline* linetodelete){
    linetodelete->pre->next=linetodelete->next;
    linetodelete->next->pre=linetodelete->pre;
    free(linetodelete->thisline->data);
    Point_Move_Col(0-linetodelete->thisline->used);
    free(linetodelete->thisline);
    point_Move_Left();
    all_buffer.current_buffer->undo_stack.push_back({4,' ',all_buffer.current_buffer->point});
    undo_iffull();
    --all_buffer.current_buffer->num_lines;
    free(linetodelete);
}
void sub_editor::New_line(storagebyline* thisline,int split_pos){
    storagebyline* newline;
    newline=(storagebyline*)malloc(sizeof(storagebyline));
    newline->thisline=(line*)malloc(sizeof(line));
    newline->pre=thisline;
    newline->next=thisline->next;
    if(thisline->next!=NULL) thisline->next->pre=newline;
    thisline->next=newline;
    if(split_pos>=thisline->thisline->used){
        newline->thisline->data=(char*)malloc(sizeof(char)*16);
        newline->thisline->length=16;
        newline->thisline->used=0;
    }
    else{
        int len=Round_Up_To_Block_Size(thisline->thisline->used-split_pos);
        newline->thisline->data=(char*)malloc(sizeof(char)*len);
        newline->thisline->length=len;
        memmove(&newline->thisline->data[0], thisline->thisline->data+split_pos, thisline->thisline->used-split_pos);
        newline->thisline->used=thisline->thisline->used-split_pos;
        thisline->thisline->used=split_pos;
    }
    ++all_buffer.current_buffer->num_lines;
    all_buffer.current_buffer->undo_stack.push_back({3,' ',all_buffer.current_buffer->point});
    undo_iffull();
}

void sub_editor::Insert_Char(char c){
    all_buffer.current_buffer->undo_stack.push_back({1,c,all_buffer.current_buffer->point});
    undo_iffull();
    all_buffer.current_buffer->point.curline->thisline=inlineinsert(all_buffer.current_buffer->point.curline->thisline,
                all_buffer.current_buffer->point.col,1,&c);
    ++all_buffer.current_buffer->num_chars;
    Point_Move_Col(1);
}
bool sub_editor::Delete(int count){
    bool ifdelline=false;
    if(count==0) return ifdelline;
    else if(count>0){
        /*if(all_buffer.current_buffer->point.curline->thisline->used==0){
            Delete_line(all_buffer.current_buffer->point.curline);
            ifdelline=true;
            --count;
        } */    //if backspace don't delete a line then use this code
        if(all_buffer.current_buffer->point.col==0){
            //backspace combine two lines
            all_buffer.current_buffer->point.curline->pre->thisline=inlineinsert(
                        all_buffer.current_buffer->point.curline->pre->thisline,
                        all_buffer.current_buffer->point.curline->pre->thisline->used,
                        all_buffer.current_buffer->point.curline->thisline->used,
                        all_buffer.current_buffer->point.curline->thisline->data);
            int pre=all_buffer.current_buffer->point.curline->thisline->used;
            Delete_line(all_buffer.current_buffer->point.curline);
            Point_Move_Col(0-pre);
            ifdelline=true;
            --count;
        }
        int count1=count;
        if(count==0) return ifdelline;
        if(count+all_buffer.current_buffer->point.col>all_buffer.current_buffer->point.curline->thisline->used){
            count=all_buffer.current_buffer->point.curline->thisline->used-all_buffer.current_buffer->point.col;
            count1-=count;
            
            inlinedelete(all_buffer.current_buffer->point.curline->thisline,
                    all_buffer.current_buffer->point.col,count);
            all_buffer.current_buffer->num_chars-=count;
            if(count1>0 && !Point_Move_Row(1)){
                Point_Move_Col(0-all_buffer.current_buffer->point.col);
                ifdelline=Delete(count1);
            } 
        }    
        else{
            all_buffer.current_buffer->undo_stack.push_back({2,
            *(all_buffer.current_buffer->point.curline->thisline->data+all_buffer.current_buffer->point.col),
            all_buffer.current_buffer->point});
            undo_iffull();
            inlinedelete(all_buffer.current_buffer->point.curline->thisline,
                        all_buffer.current_buffer->point.col,count);
            all_buffer.current_buffer->num_chars-=count;
        } 
    }
    else if(count<0){
        int count1=count;
        if(count+all_buffer.current_buffer->point.col<0){
            count=0-all_buffer.current_buffer->point.col;
            count1-=count;
            inlinedelete(all_buffer.current_buffer->point.curline->thisline,
                    all_buffer.current_buffer->point.col,count);
            all_buffer.current_buffer->num_chars+=count;
            if(!Point_Move_Row(-1)){
                Point_Move_Col(all_buffer.current_buffer->point.curline->thisline->used-all_buffer.current_buffer->point.col);
                Delete(count1);
            }
        }
        else{
            inlinedelete(all_buffer.current_buffer->point.curline->thisline,
                        all_buffer.current_buffer->point.col,count);
            all_buffer.current_buffer->num_chars+=count;
            Point_Move_Col(count);
        }
    }
    return ifdelline;
}

//*******************************************

//functions of search
bool sub_editor::search_behind(string str){
    storagebyline* p=all_buffer.current_buffer->point.curline;
    vector<int> next;
    next.resize(str.size()+1);
    int j=0,k=-1;
    next[0]=-1;
    //get next
    while(j<str.size()-1){
        if(k == -1 || str[j] == str[k]){
            j++;k++;
            if(str[j]==str[k]) next[j] = next[k];
            else next[j] = k;
        }
        else k = next[k];
    }
    int row=all_buffer.current_buffer->point.row;
    //kmp
    while(p!=NULL){
        int i;
        if(p==all_buffer.current_buffer->point.curline) i=all_buffer.current_buffer->point.col;
        else i=0;
        j=0;
        while (i < p->thisline->used && j < int(str.size())) {
            if (j == -1 || p->thisline->data[i] == str[j]) {
                i++;
                j++;
            } 
            else j = next[j];
        }
        if (j >= int(str.size())){
            location loc={row,(i - int(str.size())),p};
            Point_Set(loc);
            return true;
        }
        else{
            p=p->next;
            ++row;
        }
    }
    return false;
}
//*******************************************
//functions of undo
int sub_editor::undo(){
    if(all_buffer.current_buffer->undo_stack.empty()) return 0;
    one_undo t=all_buffer.current_buffer->undo_stack.back();
    int lastcmd=t.cmd;
    while(!all_buffer.current_buffer->undo_stack.empty() && all_buffer.current_buffer->undo_stack.back().cmd==lastcmd){
        t=all_buffer.current_buffer->undo_stack.back();
        all_buffer.current_buffer->undo_stack.pop_back();
        //for redo, push in redo stack
        all_buffer.current_buffer->redo_stack.push_back(t);
        while(all_buffer.current_buffer->redo_stack.size()>100) 
            all_buffer.current_buffer->redo_stack.pop_front();
        switch(t.cmd){
            case 1:
                all_buffer.current_buffer->point=t.loc;
                Delete(1);
                point_Move_Right();
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 2:
                all_buffer.current_buffer->point=t.loc;
                Insert_Char(t.c);
                point_Move_Left();
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 3:
                all_buffer.current_buffer->point=t.loc;
                point_Move_Left();
                Delete(1);
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 4:
                all_buffer.current_buffer->point=t.loc;
                New_line(all_buffer.current_buffer->point.curline,
                        all_buffer.current_buffer->point.col);
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
        }
    }
    return t.cmd;
}

int sub_editor::redo(){
    if(all_buffer.current_buffer->redo_stack.empty()) return 0;
    one_undo t=all_buffer.current_buffer->redo_stack.back();
    int lastcmd=t.cmd;
    while(!all_buffer.current_buffer->redo_stack.empty() && all_buffer.current_buffer->redo_stack.back().cmd==lastcmd){
        t=all_buffer.current_buffer->redo_stack.back();
        all_buffer.current_buffer->redo_stack.pop_back();
        all_buffer.current_buffer->undo_stack.push_back(t);
        undo_iffull();
        switch(t.cmd){
            case 1:
                all_buffer.current_buffer->point=t.loc;
                Insert_Char(t.c);
                point_Move_Left();
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 2:
                all_buffer.current_buffer->point=t.loc;
                Delete(1);
                point_Move_Right();
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 3:
                all_buffer.current_buffer->point=t.loc;
                New_line(all_buffer.current_buffer->point.curline,
                        all_buffer.current_buffer->point.col);
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
            case 4:
                all_buffer.current_buffer->point=t.loc;
                point_Move_Left();
                Delete(1);
                all_buffer.current_buffer->undo_stack.pop_back();
                break;
        }
    }
    return t.cmd;
}

void sub_editor::undo_iffull(){
    while(all_buffer.current_buffer->undo_stack.size()>100) 
        all_buffer.current_buffer->undo_stack.pop_front();
}
//*******************************************