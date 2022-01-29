using namespace std;
#include "constants.h"
#include "structure.h"

char* DATA_ZONE;
//extern "C" size_t my_print(int fildes, const void* buf, size_t nbytes);

//———————————————————————————扇区 分割线—————————=————————————————————

/* 从char* 输入流 读取 len 个字节，并设置扇区内容 */
void Sector::set(char* input,int len) {
	for (auto i = 0; i < SECTOR_SIZE; i++){
		if (i >= len)
			this->sector[i] = 0;
		else
			this->sector[i] = input[i];
	}
}

/**
* 根据起始扇区的第17个和第18个字节，获取BPB_RootEntCnt字段的值，也就是根目录的目录数
* 只有对起始扇区用才有意义
* ret:	int 根目录的目录数
*/
int Sector::get_BPB_RootEntCnt() {
	unsigned char a, b;
	a = this->sector[17];
	b = this->sector[18];
	//printf("a:%d\n", a);	printf("b:%d\n", b);
	return a + 256 * (b);
}

//———————————————————————————FAT 分割线—————————=——————————————
/*
* 根据文件输入流设置FAT
* 3个字节代表2个簇号
*/
void FAT12::set_FAT(fstream& fs) {
	char a, b, c;
	int i = 0;
	int cluster1, cluster2;
	while (i < SECTOR_SIZE * FAT_SECTORS) {
		fs.get(a);
		fs.get(b);
		fs.get(c);
		unsigned char ua = a, ub = b, uc = c;

		cluster1 = ua + ub % 16 * 256;
		cluster2 = ub / 16 + uc * 16;
		this->clusters.push_back(cluster1);
		this->clusters.push_back(cluster2);
		i += 3;
	}
}

/* 操作符重载,获取FAT表中索引==i 的内容 */
int& FAT12::operator[](int i) {
	return this->clusters[i];
}

/* 获取索引号为i处的内容 */
int FAT12::get(int i) {
	return this->clusters[i];
}

/**
* 获得某个Entry有关的所有簇号 
* arg: fst_clus Entry的首簇号
* ret: vector<int> Entry有关的所有簇号 
*/
vector<int> FAT12::get_all_clus(const int& fst_clus) {
	vector<int> all_clus;
	all_clus.push_back(fst_clus);
	int now_clus = fst_clus;//相当于一个FAT表的索引

	while (this->get(now_clus) <= 0x0FF7) {
		if (this->get(now_clus) == 0x0FF7) {//遇到坏簇，终止
			string bad_clus_tip = "Meet bad cluster!\n";
			//my_print(1, bad_clus_tip.c_str(), bad_clus_tip.size());
			//cout << bad_clus_tip;
			exit(0);
		}
		all_clus.push_back(this->get(now_clus));
		now_clus++;
	}
	return all_clus;
}

//———————————————————————————目录 分割线———————————————————————

/* 
*	根据字符串输入流，确定条目的content、name、attr 
*	arg:input 字符串输入流，pos:输入字符串的起始索引
*/
void Entry::set_Entry(const char* input, int pos) {
	for (auto i = 0; i < 32; ++i) {
		content.push_back(input[pos + i]);
	}
	this->attr = content[11];

	unsigned char c1 = content[26];
	unsigned char c2 = content[27];
	this->fst_clus = c1 + (c2 * 256);//c2 * 256 = c2 << 8;

	//设置name，注意不用支持LFN文件
	if (attr == 0x0F) {
		/*
		*
		* for (int i = 0; i <32; i++) {
			c = tmp[i];
			bool cond =  (i >= 1 && i < 11) | (i >= 14 && i < 26) | (i >= 28 && i < 32);
			if(c!=0&&cond&&(isalnum(c)||c=='_'))
				ret.push_back(c);
		}
		*/
	}
	else {
		for (int i = 0; i < 11; i++) {
			char c = content[i];
			if (i == 8 && attr == 0x20)
				this->name.push_back('.');
			if (isalnum(c) || c == '_'||c =='.')
				this->name.push_back(c);
		}
	}
}

/* 获取Entry某一位的内容 */
char Entry::get(int i) {
	return this->content[i];
}

char& Entry::operator[](int i) {
	return this->content[i];
}

/**
* 以调用目录为根结点，设置好 调用目录与子目录的字段 parent\childs，并且让子目录间相互连接
* 步骤;获得该目录涉及的所有簇号——获得该目录涉及的所有数据区扇区号——获得数据区扇区的索引——获得数据
* arg: input 映像文件的数据区数据，字符串输入流
*	   fat FAT12表
*/
void Entry::set_child(char* input, FAT12& fat) {
	//步骤;获得该目录涉及的所有簇号——获得该目录涉及的所有数据区扇区号+获得数据
	vector<int> clusters;
	vector<Entry*> childs;
	string dir_data;
	Entry* e;

	//获得目录的所有相关簇号
	clusters = fat.get_all_clus(this->fst_clus);

	//获得目录的所有扇区号+目录有关的所有数据；簇号——扇区号，数据区0扇区 <---> 簇号2
	for (auto i = 0; i < clusters.size(); i++) {
		int sector_no = clusters[i] - 2;
		char* start = input + sector_no * SECTOR_SIZE;
		if (dir_data.empty())
			dir_data = string(start, SECTOR_SIZE);
		else
			dir_data += string(start, SECTOR_SIZE);
	}

	//处理与目录有关的所有数据，即处理dir_data，将dir_data中的attr为0x10和0x20的转化为条目Entry
	for (auto i = 0; i < dir_data.size() / 32; i++) {
		e = new Entry;
		string one_entry = dir_data.substr(i * 32, 32);

		//如果读出来的32个byte全是0，说明已经读完了，用break跳出循环
		if (one_entry.c_str() == ZERO_32_BYTES.c_str())
			break;

		e->set_Entry(one_entry.c_str(), 0);
		if (e->attr == 0x10 || e->attr == 0x20)
			childs.push_back(e);
		
	}

	//设置彼此的link
	for (auto i = 0, j = 1; i < childs.size(); i++, j++) {
		//TODO 这里有个trick;为了便于输出，将 . 和 .. 文件夹也算在 childs里
		this->childs.push_back(childs[i]);
		if (i > 1) 
			childs[i]->parent = this;

		Entry* next = nullptr;
		if (j < childs.size()) {
			next = childs[j];
			childs[i]->succ = next;
			next->prev = childs[i];
		}
	}

	//设置子目录下的子目录,从这里开始递归
	for (auto i = 0; i < childs.size(); i++) {
		Entry* p = childs[i];
		bool cond = p->name != "." && p->name != "..";
		if (cond && p->attr == 0x10) {
			p->set_child(DATA_ZONE, fat);
		}
	}
}

//———————————————————————————目录树 分割线——————————————————————

/* 构造函数 */
Directory::Directory() {
	vector<Entry*> entries = *new vector<Entry*>;
	this->head = new Entry;
	this->head->parent = this->head;
	this->head->attr = 0x10;
}
/**
* 读入根目录区，设置目录树
* 没有考虑根目录下文件数目>224个的可能,所以要读 fst_data_sec - init_dir_sec 个扇区
* arg: input_ptr 映像文件输入流，从根目录区开始
*	   fat FAT12表
*	   fst_data_sec 数据区的第一个扇区号
*	   (init_dir_sec) 根目录区的第一个扇区号
*/
void Directory::set_root_dir(char* input_ptr, int fst_data_sec, FAT12& fat) {
	int init_dir_sec = 0 + 1 + FAT_SECTORS * 2;

	//总共读取 512 * (fst_data_sec - init_dir_sec) / 32 个目录项
	int entry_num = (fst_data_sec - init_dir_sec) * SECTOR_SIZE / ENTRY_SIZE;

	//设置根目录下的目录与文件，将之保存到 tree 里
	for (int i = 0; i < entry_num; i++) {
		Entry* tmp = new Entry;
		const char* const cp = input_ptr + i * ENTRY_SIZE;
		string arr(cp,32);

		//如果读出来的32个byte全是0，说明已经读完了，可以丢弃剩下的FAT内容，并用break跳出循环
		if (arr==ZERO_32_BYTES)
			break;

		tmp->set_Entry(arr.c_str(), 0);
		if (tmp->attr == 0x10 || tmp->attr == 0x20) {
			this->entries.push_back(tmp);

			tmp->parent = this->head;
			this->head->childs.push_back(tmp);
		}
	}

	//设置完根目录后，需要将根目录下的文件夹或者文件设置link
	//让该层的目录和文件彼此连接
	int size = this->entries.size();
	if (size > 1) {
		for (auto i = 0, j = 1; j < size; i++, j++) {
			entries[i]->succ = entries[j];
			entries[j]->prev = entries[i];
		}
	}

	DATA_ZONE = input_ptr + ENTRY_SIZE * entry_num;
	//设置根目录下的子目录,从这里开始递归
	for (auto i = 0; i < size; i++) {
		Entry* p = this->entries[i];
		if (p->attr == 0x10) {
			p->set_child(DATA_ZONE, fat);
		}
	}
}