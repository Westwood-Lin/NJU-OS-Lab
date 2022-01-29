#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <regex>
#include "structure.h"
#include "constants.h"

using namespace std;

//���� a.img; Ӳ����
const string IMGPATH = "a.img";

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


/* ���� my_print����
*  arg: fildes	�Ĵ���rsi �����
*  buf	�Ĵ���rdi ����ַ������׵�ַ
*  nbytes	�Ĵ���rdx ����ֽ���
*/
//extern "C" size_t my_print(int fildes, const void* buf, size_t nbytes);

/* ���� -l ��ls */
void no_l_print(string path, const Entry* const p) {
	vector<Entry*> entries = p->childs;

	//��·��������'/'
	if (path[path.size() - 1] != '/')
		path.push_back('/');

	//��ӡ��һ��(·��)
	string tmp = "\033[37m"+ path + ":\n";
	//my_print(1, tmp.c_str(), tmp.size());
	cout << tmp;
	tmp.clear();

	//��ӡĿ¼���ļ�
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
	//my_print(1, tmp.c_str(), tmp.size());
	cout << tmp;

	//�ݹ������Ŀ¼
	tmp = path;
	for (auto i = 0; i < entries.size(); i++) {
		Entry* parent = entries[i];
		bool cond = parent->name != "." && parent->name != "..";
		if (cond && parent->attr == 0x10) {
			tmp += parent->name + "/";
			no_l_print(tmp, parent);
		}
	}
}

/* ���㴫���Ŀ¼ p �µ�ֱ����Ŀ¼�� dir_cnt ��ֱ���ļ��� file_cnt */
void cal_cnt(const Entry* const p, int& dir_cnt, int& file_cnt) {
	if (p->attr != 0x10) {
		// "Wrong in main.cpp function cal_cnt!\n"
		cout << RES_STRINGS[2];
		//my_print(1, RES_STRINGS[2].c_str(), RES_STRINGS[2].size());
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

/* ���㴫����ļ� p ���ֽ��� */
void cal_bytes(Entry* const p,unsigned int& bytes) {
	if(p->attr!=0x20){ 
		//��ӡ"Wrong in main.cpp function cal_bytes!\n"
		cout << RES_STRINGS[3];
		//my_print(1, RES_STRINGS[3].c_str(), RES_STRINGS[3].size());
		return;
	}
	unsigned char a=p->get(28), b = p->get(29), c = p->get(30), d = p->get(31);
	bytes = a + (b << 8) + (c << 16) + (d << 24);
}

/* �� -l ��ls���� */
void l_print(string path, const Entry* const p) {
	vector<Entry*> entries = p->childs;

	if (path[path.size() - 1] != '/')
		path.push_back('/');

	int dir_cnt, file_cnt;
	unsigned int file_byte;
	cal_cnt(p,dir_cnt, file_cnt);

	//��ӡ��һ��
	string tmp = "\033[37m" + path;
	tmp += " " + to_string(dir_cnt) + " " + to_string(file_cnt) + ":\n";
	//my_print(1, tmp.c_str(), tmp.size());
	cout << tmp;
	tmp.clear();

	//��ӡֱ����Ŀ¼�����ļ���С
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
	cout << tmp;
	//my_print(1, tmp.c_str(), tmp.size());

	//�ݹ�
	tmp = path;
	for (auto i = 0; i < entries.size(); i++) {
		Entry* parent = entries[i];
		bool cond = parent->name != "." && parent->name != "..";
		if (cond && parent->attr == 0x10) {
			tmp += parent->name + "/";
			l_print(tmp, parent);
		}
	}
}

/* split ���ָ���ַ���s �� �ַ�c �ָ�����ָ��Ķδ���vector v */
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

/* ����ļ� p ������
* arg:	p:Ҫ������ļ���Ŀ
*		fat:FAT��Ҫ���ļ������дغ�
*		data:ӳ���ļ�a.img��������
*/
void cat_file(const Entry* const p,FAT12 fat,const char* const data) {
	//���������صĴغš������������������,������string�С������
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
	cout << str;
	//my_print(1, str.c_str(), str.size());
}

/* 
* ����������ļ�·�� path
* arg:
*   path �ļ�·�� head Ŀ¼���ĸ����
*	1.��·����Сд��ĸȫ��תΪ��д 2.�������·����'/'���ֳɶ�	3.��·���淶������
* ret:
*	nullptr û��ƥ�䵽
*	Entry* p;�ļ�����Ŀָ��
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

	//һ����ʼ�������ļ�
	/*
	* �ֲ���������
	* fs:�ļ�������
	* tips:��ʾ��������
	* input:��������������Ĳ���
	* cmd:������
	* iss:�ַ�������,���������ȽϷ��� ( >>�����̫ǿ����
	* fat:FAT�����ݽṹ��vector<int>;int����غ�
	* tree:��Ŀ¼���ݽṹ������vector<Entry*>;
	* fst_data_sec:��һ����������������
	* Sector boot_sector; ��������
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
	* ���������ļ�����������������ʼ������FAT12����Ŀ¼��;
	* fs.read(buff,510)����Ϊ��windows��VS2019�ϳ��Զ�ȡʱ��ʼ�������ڶ������⣬����510���ֽڲ���Ӱ�����
	*	ע�⣡ linux������͵ø�Ϊfs.read(buff,512)
	* fst_data_sec:����������ʼ������,�����ù����У�������������;
	* set_FAT:����FAT1���������ݱ���;
	* fs.ignore(FAT_SECTORS * SECTOR_SIZE);:����FAT2;
	* 
	* set_directory_area:������Ŀ¼�������ú�Ŀ¼��;
	* buff += �� �ڱ����Ժ�ͨ���޸�ָ�붪����Ŀ¼��
	*/
	fs.read(buff, 510);//TODO! linux������������� fs.read(buff, 512)! ���ܸ����������������ƽ̨�����й�
	boot.set(buff,510);

	fst_data_sec = 0 + 1 + FAT_SECTORS * 2 + (boot.get_BPB_RootEntCnt() / 16);

	fat.set_FAT(fs);
	fs.ignore(FAT_SECTORS * SECTOR_SIZE);

	fs.read(buff, SECTOR_SIZE * (2880 - 1 - FAT_SECTORS * 2));
	tree.set_root_dir(buff, fst_data_sec,fat);
	buff += boot.get_BPB_RootEntCnt() * ENTRY_SIZE;


	//����ѭ����������,������Ĳ���


	while (true){

		/*
		* ��ʾ��������,��ȡÿ������input,�����ַ���iss
		*/
		cout << tips;
		//my_print(1, t\ips.c_str(), tips.size());
		getline(cin, input);
		iss.str(input);
		
		//�ж��Ƿ�ֻ�����˿��ַ�(\t \n space)
		if (!(iss >> cmd)) {
			//"No valid input\n"
			//my_print(1, RES_STRINGS[4].c_str(), RES_STRINGS[4].size());
			cout << RES_STRINGS[4];
		}

		//exit ��ʾ+��ѭ��
		else if (cmd == "exit"){
			//"Program ends\n",
			//my_print(1,RES_STRINGS[0].c_str(), RES_STRINGS[0].size());
			cout << RES_STRINGS[0];
			break;
		}

		//ls
		else if (cmd == "ls") {
			/*
			* ��������������������1.�����ʾ��Ϣ����ʼ����������������������
			* ls���������2�����õĺ���������1����-l��1����Ŀ¼
			* has_dir:�Ƿ���е�ַ����
			* has_l:�Ƿ����-l ����
			* dir:��ַ��Ĭ��Ϊӳ���ļ��ĸ�Ŀ¼
			* wrong_param: ���������Ƿ���ȷ
			*/
			string tmp;
			bool has_dir = false;
			bool has_l = false;
			bool right_param = true;
			string path = "/";

			/* 
			* ��������������������2.���� ls ��������������������������������
			* ְ��: ���úñ��� has_dir��has_l��dir
			* option:����ƥ�� -l ��pattern��ֻ���� ��-����ͷ�����������1��l��Ҳ�����������'+'
			* dir_path:����ƥ�� Ŀ¼ ��pattern���� "./"��ͷ,������ "/"��ͷ
			*/
			regex option( "-l+");
			regex dir_path("(/.*)|(\\./.*)|(\\./.*)|(\\.\\./.*)");
			while (iss >> tmp) {
				//-l+ ;�����и� !has_l ��˼������������-l,�����-l ����ֱ�����ӣ��ӿ촦���ٶ�
				if (!has_l && regex_match(tmp, option)) {
					has_l = true;
					//my_print(1, "ls receive -l option\n", 22);
				}

				//����Ѿ�������Ŀ¼����������һ�μ�鵽��Ŀ¼������ֱ�ӱ���
				else if (has_dir && regex_match(tmp, dir_path)) {
					//"Error!ls receives more than 1 directory\n"
					//my_print(1, RES_STRINGS[5].c_str(), RES_STRINGS[5].size());
					cout << RES_STRINGS[5];
					break;
				}

				//û������Ŀ¼����鵽��Ŀ¼����
				else if (!has_dir&&regex_match(tmp, dir_path)) {
					has_dir = true;
					path = tmp;
				}

				//��鵽�˲�֧�ֵĲ��� �� -t -x 
				else {
					right_param = false;
					break;
				}
			}
			/*
			*��������������������3.�����Ϣ��������������������������������-
			* ��right_param:��֧�ֵĺ�������
			*/
			if (!right_param)
				//"Unsuppoted parameter!\n"
				//my_print(1, RES_STRINGS[6].c_str(), RES_STRINGS[6].size());
				cout << RES_STRINGS[6];


			else if (!has_l) {
				const Entry* e= parse_path(path,tree.head);
				if (e != nullptr)
					no_l_print(path, e);
				else
					//"Can't find target\n";
					//my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
					cout << RES_STRINGS[7];
			}

			else if (has_l) {
				const Entry* e = parse_path(path, tree.head);
				if (e != nullptr)
					l_print(path, e);
				else
					//"Can't find target\n";
					//my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
					cout << RES_STRINGS[7];

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
					//my_print(1, tmp.c_str(), tmp.size());
					cout << tmp;
					
					cat_file(p,fat,buff);
				}
				else {
					//"Can't find target\n";
					//my_print(1, RES_STRINGS[7].c_str(), RES_STRINGS[7].size());
					cout << RES_STRINGS[7];
				}
			}
			else {
				//"missing file path or file name!\n"
				//my_print(1, RES_STRINGS[8].c_str(), RES_STRINGS[8].size());
				cout << RES_STRINGS[8];

			}
		}
		//��֧�ֵ�����
		else {
			//"Unsupported command!\n",
			//my_print(1, RES_STRINGS[1].c_str(), RES_STRINGS[1].size());
			cout << RES_STRINGS[1];
		}

		/*
		* ������һ������
		* ����ַ���iss;���һ������input;��ձ�������ı���cmd
		*/

		iss.clear();
		input.clear();
		cmd.clear();
	}

	fs.close();
	return 0;
}