#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <regex>
#include "structure.h"
#include "constants.h"

using namespace std;

//软盘 a.img; 硬编码
const string IMGPATH = "a2.img";

string RES_STRINGS[] = {
"Program ends\n",
"Unsupported command!\n",
"Wrong in main.cpp function cal_cnt!\n",
"Wrong in main.cpp function cal_bytes!\n",
"No valid input\n",
"Error!ls receives more than 1 directory\n",
"Unsuppoted parameter!\n",
"Can't find target\n",
"missing file path or file name!\n",
};


/* 声明 my_print函数
*  arg: fildes	寄存器rsi 输出流
*	buf	寄存器rdi 输出字符串的首地址
*	nbytes	寄存器rdx 输出字节数
*/
extern "C" size_t my_print(int fildes, const void* buf, size_t nbytes);

/* 不带 -l 的ls */
void no_l_print(string path, const Entry* const p) {
	vector<Entry*> entries = p->childs;

	//在路径名最后加'/'
	if (path[path.size() - 1] != '/')
		path.push_back('/');

	//打印第一行(路径)
	string tmp = "\033[37m"+ path + ":\n";
	my_print(1, tmp.c_str(), tmp.size());
	tmp.clear();

	//打印目录内文件
	for (int i = 0; i < entries.size(); i++) {
		if (entries[i]->attr == 0x10) {
			tmp += "\033[31m" + entries[i]->name;
		}
		else
			tmp += "\033[37m"+ entries[i]->name;
		if (i == entries.size() - 1)
			tmp += "\n";
		else
			tmp += " ";
	}
	my_print(1, tmp.c_str(), tmp.size());
	tmp.clear();

	//递归输出子目录
	//tmp = path;
	for (auto i = 0; i < entries.size(); i++) {
		tmp = path;
		Entry* parent = entries[i];
		bool cond = parent->name != "." && parent->name != "..";
		if (cond && parent->attr == 0x10) {
			tmp += parent->name + "/";		
			no_l_print(tmp, parent);
		}
	}
}

/* 计算传入的目录 p 下的直接子目录数 dir_cnt 和直接文件数 file_cnt */
void cal_cnt(const Entry* const p, int& dir_cnt, int& file_cnt) {
	if (p->attr != 0x10) {
		// "Wrong in main.cpp function cal_cnt!\n"
		my_print(1, RES_STRINGS[2].c_str(), RES_STRINGS[2].size());
		return;
	}

	dir_cnt = 0;
	file_cnt = 0;
	for (auto i = 0; i < p->childs.size(); i++){
		bool cond = (p->childs[i]->name != ".") && (p->childs[i]->name != "..");
		if (cond) {
			if (p->childs[i]->attr == 0x10) {
				dir_cnt++;
			}
			else if (p->childs[i]->attr == 0x20) {
				file_cnt++;
			}
		}
	}
}

/* 计算传入的文件 p 的字节数 */
void cal_bytes(Entry* const p,unsigned int& bytes) {
	if(p->attr!=0x20){ 
		//打印"Wrong in main.cpp function cal_bytes!\n"
		my_print(1, RES_STRINGS[3].c_str(), RES_STRINGS[3].size());
		return;
	}
	unsigned char a=p->get(28), b = p->get(29), c = p->get(30), d = p->get(31);
	bytes = a + (b << 8) + (c << 16) + (d << 24);
}

/* 带 -l 的ls命令 */
void l_print(string path, const Entry* const p) {
	vector<Entry*> entries = p->childs;

	if (path[path.size() - 1] != '/')
		path.push_back('/');

	int dir_cnt, file_cnt;
	unsigned int file_byte;
	cal_cnt(p,dir_cnt, file_cnt);

	//打印第一行
	string tmp = "\033[37m" + path;
	tmp += " " + to_string(dir_cnt) + " " + to_string(file_cnt) + ":\n";
	my_print(1, tmp.c_str(), tmp.size());
	tmp.clear();

	//打印直接子目录数和文件大小
	for (int i = 0; i < entries.size(); i++) {
		if (entries[i]->attr == 0x10) {
			tmp += "\033[31m" + entries[i]->name + " ";
			cal_cnt(entries[i], dir_cnt, file_cnt);
			tmp += "\033[37m" + to_string(dir_cnt) + " " + to_string(file_cnt);
		}
		else if(entries[i]->attr == 0x20) {
			cal_bytes(entries[i], file_byte);
			tmp += "\033[37m" + entries[i]->name + " " + to_string(file_byte);
		}
		tmp += "\n";
	}
	tmp += "\n";
	my_print(1, tmp.c_str(), tmp.size());
	tmp.clear();
	//递归
	//tmp = path;
	for (auto i = 0; i < entries.size(); i++) {
		tmp = path;
		Entry* parent = entries[i];
		bool cond = parent->name != "." && parent->name != "..";
		if (cond && parent->attr == 0x10) {
			tmp += parent->name + "/";
			l_print(tmp, parent);
		}
	}
}

/* split 串分割，将字符串s 按 字符c 分割，并将分割后的段存入vector v */
void split(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2){
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
	for (auto i = 0; i < v.size(); ) {
		if (v[i].size() == 0)
			v.erase(v.begin() + i);
		else
			i++;
	}

}

/* 输出文件 p 的内容
* arg:	p:要输出的文件条目
*		fat:FAT表，要查文件的所有簇号
*		data:映像文件a.img的数据区
*/
void cat_file(const Entry* const p,FAT12 fat,const char* const data) {
	//获得所有相关的簇号——获得所有扇区内容,保存在string中——输出
	vector<int> clusters = fat.get_all_clus(p->fst_clus);
	string str;
	for (size_t i = 0; i < clusters.size(); i++){
		int sec_no = clusters[i] - 2;
		if (str.empty())
			str = string(data + sec_no * SECTOR_SIZE, SECTOR_SIZE);
		else {
			string tmp(data + sec_no * SECTOR_SIZE, SECTOR_SIZE);
			str.insert(str.end(), tmp.begin(), tmp.end());
		}
	}
	str += "\n";
	my_print(1, str.c_str(), str.size());
}

/* 
* 解析传入的文件路径 path
* arg:
*   path 文件路径 head 目录树的根结点
*	1.将路径的小写字母全部转为大写 2.将输入的路径按'/'划分成段	3.将路径规范化处理
* ret:
*	nullptr 没有匹配到
*	Entry* p;文件的条目指针
*/
const Entry* parse_path(string& path, const Entry* const head) {
	vector<string> seg;
	for (int i = 0; i < path.size(); i++) {
		if (path[i] <= 'z' && path[i] >= 'a')
			path[i] = toupper(path[i]);
	}

	split(path, seg, "/");
	path.clear();

	const Entry* p = head;
	bool matched = true;
	for (auto i = 0; i < seg.size(); i++) {
		path.push_back('/');
		if (seg[i] == ".") {
			path.push_back('.');
		}
		else if (seg[i] == "..") {
			p = p->parent;
			path.push_back('.');
			path.push_back('.');
		}
		else {
			for (auto j = 0; j < p->childs.size(); j++) {
				if (seg[i] == p->childs[j]->name) {
					p = p->childs[j];
					path.insert(path.end(), seg[i].begin(), seg[i].end());
					matched = true;
					break;
				}
				matched = false;
			}
		}
	}
	if (!matched)
		return nullptr;
	if (path.size() == 0) {
		path = "/";
		return head;
	}
	return p;
}

int main(void) {

		//一、初始化、打开文件
	/*
	* 局部变量声明
	* fs:文件输入流
	* tips:提示输入命令
	* input:保存命令行输入的参数
	* cmd:命令项
	* iss:字符流输入,操作起来比较方便 ( >>运算符太强大了
	* fat:FAT表数据结构，vector<int>;int代表簇号
	* tree:根目录数据结构，包含vector<Entry*>;
	* fst_data_sec:第一个数据区的扇区号
	* Sector boot_sector; 引导扇区
	* char* buff = new char[SECTOR_SIZE * 2880]; 
	*/
	fstream fs;
	fs.open(IMGPATH, ios::in);
	string tips = "\n\033[37mPlease input any command below: \nls [-l] [path] OR cat filename OR exit\n";

	string input;
	string cmd;
	istringstream iss;

	FAT12 fat;
	Directory tree;
	int fst_data_sec;

	Sector boot;
	char* buff = new char[SECTOR_SIZE * 2880];

/*
* 遍历镜像文件，设置数据区的起始扇区、FAT12表、根目录区;
* 注意！
* linux上这里是fs.read(buff,512) 没有问题
* 在windows的VS2019上尝试读取起始扇区时存在对齐问题，调试后修改成fs.read(buff,510)
*	在windows上读入510个字节才是正确的
* 
* fst_data_sec:数据区的起始扇区号,在设置过程中，遍历引导扇区;
* set_FAT:遍历FAT1，并将数据保存;
* fs.ignore(FAT_SECTORS * SECTOR_SIZE):丢弃FAT2;
* set_directory_area:遍历根目录区并设置好目录树;
* buff += … 在遍历以后，通过修改指针丢弃根目录区,最后剩下数据区
*/
	fs.read(buff, SECTOR_SIZE);//TODO! linux上这里fs.read(buff,512) 没有问题
	boot.set(buff,SECTOR_SIZE);

	fst_data_sec = 0 + 1 + FAT_SECTORS * 2 + (boot.get_BPB_RootEntCnt() / 16);

	fat.set_FAT(fs);
	fs.ignore(FAT_SECTORS * SECTOR_SIZE);

	fs.read(buff, SECTOR_SIZE * (2880 - 1 - FAT_SECTORS * 2));
	tree.set_root_dir(buff, fst_data_sec,fat);
	buff += boot.get_BPB_RootEntCnt() * ENTRY_SIZE;


	//二、循环处理输入,程序核心部分

	while (true){

		/*
		* 提示输入命令,获取每行输入input,设置字符流iss
		*/
		my_print(1, tips.c_str(), tips.size());
		getline(cin, input);
		iss.str(input);
		
		//判断是否只输入了空字符(\t \n space)
		if (!(iss >> cmd)) {
			//"No valid input\n"
			my_print(1, RES_STRINGS[4].c_str(), RES_STRINGS[4].size());
		}

		//exit 提示+出循环
		else if (cmd == "exit"){
			//"Program ends\n",
			my_print(1,RES_STRINGS[0].c_str(), RES_STRINGS[0].size());
			break;
		}

		//ls
		else if (cmd == "ls") {
			/*
			* ——————————1.输出提示信息、初始化——————————
			* ls命令最多有2个有用的后续参数：1个是-l，1个是目录
			* has_dir:是否带有地址参数
			* has_l:是否带有-l 参数
			* dir:地址，默认为映像文件的根目录
			* wrong_param: 后续参数是否正确
			*/
			string tmp;
			bool has_dir = false;
			bool has_l = false;
			bool right_param = true;
			string path = "/";

			/* 
			* ——————————2.处理 ls 附带参数————————————
			* 职责: 设置好变量 has_dir、has_l、dir
			* option:正则匹配 -l 的pattern，只能是 ‘-’开头，后面跟至少1个l，也就是正则里的'+'
			* dir_path:正则匹配 目录 的pattern，以 "./"开头,或者以 "/"开头
			*/
			regex option( "-l+");
			regex dir_path("(/.*)|(\\./.*)|(\\./.*)|(\\.\\./.*)");
			while (iss >> tmp) {
			//-l+ ;这里有个 !has_l 意思是如果多次输入-l,后面的-l 可以直接无视，加快处理速度
				if (!has_l && regex_match(tmp, option)) {
					has_l = true;
					//my_print(1, "ls receive -l option\n", 22);
				}

				//如果已经设置了目录参数，但又一次检查到了目录参数，直接报错
				else if (has_dir && regex_match(tmp, dir_path)) {
					//"Error!ls receives more than 1 directory\n"
					my_print(1, RES_STRINGS[5].c_str(), RES_STRINGS[5].size());
					break;
				}

				
				//没有设置目录，检查到了目录参数
				else if (!has_dir&&regex_match(tmp, dir_path)) {
					has_dir = true;
					path = tmp;
				}

				//检查到了不支持的参数 如 -t -x 
				else {
					right_param = false;
					break;
				}
			}
			/*
			* ——————————3.输出信息————————————————-
			* ！right_param:不支持的后续参数
			*/
			if (!right_param)
				//"Unsuppoted parameter!\n"
				my_print(1, RES_STRINGS[6].c_str(), RES_STRINGS[6].size());

			else if (!has_l) {
				const Entry* e= parse_path(path,tree.head);
				if (e != nullptr)
					no_l_print(path, e);
				else
					//"Can't find target\n";
					my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
			}

			else if (has_l) {
				const Entry* e = parse_path(path, tree.head);
				if (e != nullptr)
					l_print(path, e);
				else
					//"Can't find target\n";
					my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
			}
		}

		//cat
		else if (cmd == "cat") {
			string file;
			if (iss >> file) {
				const Entry * p = parse_path(file, tree.head);
				if (p != nullptr) {
					string tmp("Successfully read file:");
					tmp += p->name + "\n";
					my_print(1, tmp.c_str(), tmp.size());
					
					cat_file(p,fat,buff);
				}
				else {
					//"Can't find target\n";
					my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
				}
			}
			else {
				//"missing file path or file name!\n"
				my_print(1, RES_STRINGS[8].c_str(), RES_STRINGS[8].size());
			}
		}
		//不支持的命令
		else {
			//"Unsupported command!\n",
			my_print(1, RES_STRINGS[1].c_str(), RES_STRINGS[1].size());
		}

		/*
		* 处理完一次命令
		* 清空字符流iss;清空一行输入input;清空保存命令的变量cmd
		*/

		iss.clear();
		input.clear();
		cmd.clear();
	}

	fs.close();
	return 0;
}
