# ifndef   _Golay_H
#   define _Golay_H


extern "C++" class CGolayCodec {    // В оригинале без extern "C++"
    // class CGolayCodec {
 public:
	CGolayCodec();	//�����������(�������������� �������)
	void encode(char *mess, char *cword);	//��������� �����������
	void decode(double *cword, char *mess);	//��������� �������������
	const static int LENGTHWORD = 12;	//����� ����������� ���������
	const static int LENGTHCWORD = 24;	//����� �������� �����

private:
	const static int DEEP = 4;	//����� ������� ������ ����� �������
	const static int EDGE = 2;	//����� �����, ��������� � �������
	const static int STATES = 16;	//������ �������(����� �������)

	typedef struct	//����������� ���� "������� �������"
	{
		int zero[EDGE];	//���� �� ������� �����, ��������� �� ������ �������
		int unit[EDGE];	//���� �� ������ �����, ��������� �� ������ �������
		int zPath;	//����� ������, � ������� ����� ������� �����
		int uPath;	//����� ������, � ������� ����� ������ �����
	}trellisType; 
	trellisType trellis[STATES][DEEP]; //������� "������"
	typedef struct	//����������� ���� "����"
	{
		char dWord[LENGTHWORD];	//������� ���� ��������������� ����� �� ������� "����"
		double dist;	//������� ����������
		bool flag;	//�������� �� �� ������ ���� ���� ��� ����� ������
	}pathType; 
	pathType pathes[STATES];	//������ ������������� �����
	pathType prevPathes[STATES];	//������ ����� �� ���������� ����
	double sqr(double x);	//���������� � �������
	void copyPathes(int k);	//����������� �����
};

# endif