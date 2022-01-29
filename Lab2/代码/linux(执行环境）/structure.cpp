using namespace std;
#include "constants.h"
#include "structure.h"

char* DATA_ZONE;
extern "C" size_t my_print(int fildes, const void* buf, size_t nbytes);

//���������������������������������������������������������� �ָ��ߡ�����������������=����������������������������������������

/* ��char* ������ ��ȡ len ���ֽڣ��������������� */
void Sector::set(char* input,int len) {
	for (auto i = 0; i < SECTOR_SIZE; i++){
		if (i >= len)
			this->sector[i] = 0;
		else
			this->sector[i] = input[i];
	}
}

/**
* ������ʼ�����ĵ�17���͵�18���ֽڣ���ȡBPB_RootEntCnt�ֶε�ֵ��Ҳ���Ǹ�Ŀ¼��Ŀ¼��
* ֻ�ж���ʼ�����ò�������
* ret:	int ��Ŀ¼��Ŀ¼��
*/
int Sector::get_BPB_RootEntCnt() {
	unsigned char a, b;
	a = this->sector[17];
	b = this->sector[18];
	//printf("a:%d\n", a);	printf("b:%d\n", b);
	return a + 256 * (b);
}

//������������������������������������������������������FAT �ָ��ߡ�����������������=����������������������������
/*
* �����ļ�����������FAT
* 3���ֽڴ���2���غ�
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

/* ����������,��ȡFAT��������==i ������ */
int& FAT12::operator[](int i) {
	return this->clusters[i];
}

/* ��ȡ������Ϊi�������� */
int FAT12::get(int i) {
	return this->clusters[i];
}

/**
* ���ĳ��Entry�йص����дغ� 
* arg: fst_clus Entry���״غ�
* ret: vector<int> Entry�йص����дغ� 
*/
vector<int> FAT12::get_all_clus(const int& fst_clus) {
	vector<int> all_clus;
	all_clus.push_back(fst_clus);
	int now_clus = fst_clus;//�൱��һ��FAT�������

	while (this->get(now_clus) <= 0x0FF7) {
		if (this->get(now_clus) == 0x0FF7) {//�������أ���ֹ
			string bad_clus_tip = "Meet bad cluster!\n";
			my_print(1, bad_clus_tip.c_str(), bad_clus_tip.size());
			exit(0);
		}
		all_clus.push_back(this->get(now_clus));
		now_clus++;
	}
	return all_clus;
}

//������������������������������������������������������Ŀ¼ �ָ��ߡ���������������������������������������������

/* 
*	�����ַ�����������ȷ����Ŀ��content��name��attr 
*	arg:input �ַ�����������pos:�����ַ�������ʼ����
*/
void Entry::set_Entry(const char* input, int pos) {
	for (auto i = 0; i < 32; ++i) {
		content.push_back(input[pos + i]);
	}
	this->attr = content[11];

	unsigned char c1 = content[26];
	unsigned char c2 = content[27];
	this->fst_clus = c1 + (c2 * 256);//c2 * 256 = c2 << 8;

	//����name��ע�ⲻ��֧��LFN�ļ�
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

/* ��ȡEntryĳһλ������ */
char Entry::get(int i) {
	return this->content[i];
}

char& Entry::operator[](int i) {
	return this->content[i];
}

/**
* �Ե���Ŀ¼Ϊ����㣬���ú� ����Ŀ¼����Ŀ¼���ֶ� parent\childs����������Ŀ¼���໥����
* ����;��ø�Ŀ¼�漰�����дغš�����ø�Ŀ¼�漰�����������������š���������������������������������
* arg: input ӳ���ļ������������ݣ��ַ���������
*	   fat FAT12��
*/
void Entry::set_child(char* input, FAT12& fat) {
	//����;��ø�Ŀ¼�漰�����дغš�����ø�Ŀ¼�漰������������������+�������
	vector<int> clusters;
	vector<Entry*> childs;
	string dir_data;
	Entry* e;

	//���Ŀ¼��������شغ�
	clusters = fat.get_all_clus(this->fst_clus);

	//���Ŀ¼������������+Ŀ¼�йص��������ݣ��غš��������ţ�������0���� <---> �غ�2
	for (auto i = 0; i < clusters.size(); i++) {
		int sector_no = clusters[i] - 2;
		char* start = input + sector_no * SECTOR_SIZE;
		if (dir_data.empty())
			dir_data = string(start, SECTOR_SIZE);
		else
			dir_data += string(start, SECTOR_SIZE);
	}

	//������Ŀ¼�йص��������ݣ�������dir_data����dir_data�е�attrΪ0x10��0x20��ת��Ϊ��ĿEntry
	for (auto i = 0; i < dir_data.size() / 32; i++) {
		e = new Entry;
		string one_entry = dir_data.substr(i * 32, 32);

		//�����������32��byteȫ��0��˵���Ѿ������ˣ���break����ѭ��
		if (one_entry.c_str() == ZERO_32_BYTES.c_str())
			break;

		e->set_Entry(one_entry.c_str(), 0);
		if (e->attr == 0x10 || e->attr == 0x20)
			childs.push_back(e);
		
	}

	//���ñ˴˵�link
	for (auto i = 0, j = 1; i < childs.size(); i++, j++) {
		//TODO �����и�trick;Ϊ�˱���������� . �� .. �ļ���Ҳ���� childs��
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

	//������Ŀ¼�µ���Ŀ¼,�����￪ʼ�ݹ�
	for (auto i = 0; i < childs.size(); i++) {
		Entry* p = childs[i];
		bool cond = p->name != "." && p->name != "..";
		if (cond && p->attr == 0x10) {
			p->set_child(DATA_ZONE, fat);
		}
	}
}

//������������������������������������������������������Ŀ¼�� �ָ��ߡ�������������������������������������������

/* ���캯�� */
Directory::Directory() {
	vector<Entry*> entries = *new vector<Entry*>;
	this->head = new Entry;
	this->head->parent = this->head;
	this->head->attr = 0x10;
}
/**
* �����Ŀ¼��������Ŀ¼��
* û�п��Ǹ�Ŀ¼���ļ���Ŀ>224���Ŀ���,����Ҫ�� fst_data_sec - init_dir_sec ������
* arg: input_ptr ӳ���ļ����������Ӹ�Ŀ¼����ʼ
*	   fat FAT12��
*	   fst_data_sec �������ĵ�һ��������
*	   (init_dir_sec) ��Ŀ¼���ĵ�һ��������
*/
void Directory::set_root_dir(char* input_ptr, int fst_data_sec, FAT12& fat) {
	int init_dir_sec = 0 + 1 + FAT_SECTORS * 2;

	//�ܹ���ȡ 512 * (fst_data_sec - init_dir_sec) / 32 ��Ŀ¼��
	int entry_num = (fst_data_sec - init_dir_sec) * SECTOR_SIZE / ENTRY_SIZE;

	//���ø�Ŀ¼�µ�Ŀ¼���ļ�����֮���浽 tree ��
	for (int i = 0; i < entry_num; i++) {
		Entry* tmp = new Entry;
		const char* const cp = input_ptr + i * ENTRY_SIZE;
		string arr(cp,32);

		//�����������32��byteȫ��0��˵���Ѿ������ˣ����Զ���ʣ�µ�FAT���ݣ�����break����ѭ��
		if (arr==ZERO_32_BYTES)
			break;

		tmp->set_Entry(arr.c_str(), 0);
		if (tmp->attr == 0x10 || tmp->attr == 0x20) {
			this->entries.push_back(tmp);

			tmp->parent = this->head;
			this->head->childs.push_back(tmp);
		}
	}

	//�������Ŀ¼����Ҫ����Ŀ¼�µ��ļ��л����ļ�����link
	//�øò��Ŀ¼���ļ��˴�����
	int size = this->entries.size();
	if (size > 1) {
		for (auto i = 0, j = 1; j < size; i++, j++) {
			entries[i]->succ = entries[j];
			entries[j]->prev = entries[i];
		}
	}

	DATA_ZONE = input_ptr + ENTRY_SIZE * entry_num;
	//���ø�Ŀ¼�µ���Ŀ¼,�����￪ʼ�ݹ�
	for (auto i = 0; i < size; i++) {
		Entry* p = this->entries[i];
		if (p->attr == 0x10) {
			p->set_child(DATA_ZONE, fat);
		}
	}
}