#pragma once
using namespace std;
/* ���ļ�����: 4�����ݽṹ:������FAT12��Ŀ¼������㣩��Ŀ¼�������� */
//���������������������������������������������������������� �ָ��ߡ�����������������=����������������������������
/**
* ���� Sector
*		��Ա����: char sector[512];��Ҫ����Ϊʲô���� private
*		�ṩ����: void set(char*,int);;  ������������
*				  int get_BPB_RootEntCnt();  ����������RootEntCnt����Ŀ¼��,���ڼ����Ŀ¼����С
*/

class Sector
{
public:
	void set(char*,int);
	int get_BPB_RootEntCnt();
	char sector[512];
private:
};

//������������������������������������������������������FAT �ָ��ߡ�����������������=������������������������������
/**
* FAT12
*		��Ա����: vector<int> clusters;
*		�ṩ����: void set_FAT(fstream& fs); ����������ȷ��FAT������
*				  int& operator[](int); ����������,��ȡ����������
*				  int get(int);��ȡ����������
*				  vector<int>& get_all_clus(const int&); �����״غ�,���ĳ�� Entry �����дغ�
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

//������������������������������������������������������Ŀ¼�� �ָ��ߡ���������������������������������������������
class Entry;

/* 
* Ŀ¼��
* ��Ա����:
*			head �����
*			entries ��Ŀ¼�µ��ļ�;��ʵ����head->childs;
* ����: 	
*			Entry* set_root_dir:����������ַ���char* ����һ�����������š� FAT��,���ú�Ŀ¼���ĸ������ӹ�ϵ;
*			Directory();���캯��,��Ҫ�ǳ�ʼ��head
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
* ����㡪���ļ�/Ŀ¼ Entry
*		��Ա����:
*				  string content; 32λ��Ŀ¼����
*				  string name; �ļ���Ŀ¼��
*				  char attr; ����:0x10Ŀ¼ 0x20�ļ�
*				  int fst_clus ;�״غ�
*				  vector<Entry*> childs;Ŀ¼����Ů���
*				  Entry* parent; �����
*				  Entry* prev; ǰһ���ֵܽ��
*				  Entry* succ; ��һ���ֵܽ��
*		�ṩ����:
*				  void set_Entry(const char*,int); ����content��name��attr
*				  char get(int);	char& operator[](int);��ȡcontent����
*				  set_child; �ݹ������Ŀ¼�ļ�������ָ��;childs��parent��prev��succ;
*							 �����Ŀ¼�����ļ���Ŀ¼��������Ŀ¼�ݹ����set_child
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
