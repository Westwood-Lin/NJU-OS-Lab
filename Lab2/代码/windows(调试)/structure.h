#pragma once
using namespace std;
/* 该文件包括: 4个数据结构:扇区、FAT12、目录（树结点）、目录树的声明 */
//———————————————————————————扇区 分割线—————————=——————————————
/**
* 扇区 Sector
*		成员变量: char sector[512];不要在意为什么不是 private
*		提供方法: void set(char*,int);;  设置扇区内容
*				  int get_BPB_RootEntCnt();  返回扇区的RootEntCnt根的目录数,便于计算根目录区大小
*/

class Sector
{
public:
	void set(char*,int);
	int get_BPB_RootEntCnt();
	char sector[512];
private:
};

//———————————————————————————FAT 分割线—————————=———————————————
/**
* FAT12
*		成员变量: vector<int> clusters;
*		提供方法: void set_FAT(fstream& fs); 根据输入流确定FAT表内容
*				  int& operator[](int); 操作符重载,获取索引处内容
*				  int get(int);获取索引处内容
*				  vector<int>& get_all_clus(const int&); 根据首簇号,获得某个 Entry 的所有簇号
*/
class FAT12
{
public:
	FAT12() {};
	~FAT12() {
		clusters.clear();
	}
	void set_FAT(fstream&);
	int& operator[](int);
	int get(int);
	vector<int> get_all_clus(const int&);
private:
	vector<int> clusters;
};

//———————————————————————————目录树 分割线———————————————————————
class Entry;

/* 
* 目录树
* 成员变量:
*			head 根结点
*			entries 根目录下的文件;其实就是head->childs;
* 方法: 	
*			Entry* set_root_dir:根据输入的字符串char* 、第一个数据扇区号、 FAT表,设置好目录树的各种链接关系;
*			Directory();构造函数,主要是初始化head
*/
class Directory
{
public:
	Directory();
	void set_root_dir(char*, int, FAT12&);
	vector<Entry*> entries;
	Entry* head;
private:
};

/**
* 树结点——文件/目录 Entry
*		成员变量:
*				  string content; 32位的目录内容
*				  string name; 文件或目录名
*				  char attr; 类型:0x10目录 0x20文件
*				  int fst_clus ;首簇号
*				  vector<Entry*> childs;目录的子女结点
*				  Entry* parent; 父结点
*				  Entry* prev; 前一个兄弟结点
*				  Entry* succ; 后一个兄弟结点
*		提供方法:
*				  void set_Entry(const char*,int); 设置content、name、attr
*				  char get(int);	char& operator[](int);获取content内容
*				  set_child; 递归地设置目录文件的四类指针;childs、parent、prev、succ;
*							 如果该目录的子文件有目录，将对子目录递归调用set_child
*/
class Entry {
public:
	void set_Entry(const char*, int);
	char get(int);
	char& operator[](int);
	void set_child(char*, FAT12&);

	Entry* parent = nullptr;
	vector<Entry*> childs;
	Entry* prev = nullptr;
	Entry* succ = nullptr;
	string name;
	char attr;
	int fst_clus;
private:
	string content;
};
