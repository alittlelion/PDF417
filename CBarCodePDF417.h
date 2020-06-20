#pragma once
/**
* \brief
*Function: PDF417 encode
*Editor: Mr.Wu
*Timer: 2019-8-12
*/

#include <string>
#include <vector>

#include "CPDF417.h"


class  CBarCodePDF417
{
public:
	CBarCodePDF417();
	CBarCodePDF417(std::string text);
	~CBarCodePDF417();

	/**
	*\brief
	*设置 PDF417 编码数据
	*/
	void setText(std::string text);

	/**
	*\brief
	*设置纠错等级 0 - 8 级
	*/
	void setErrorLevel(int level);


	/**
	*\brief
	*BC 模式编码
	*/
	void ByteCompress(const unsigned char *str,int length);

	/**
	*\brief
	*TC 模式编码
	*/
	void TextCompress(const unsigned char *str, int length);

	/**
	*\brief
	*NC 模式编码
	*/
	void NumberCompress(const unsigned char *str,int length);

	/**
	*\brief
	*文本压缩模式: 解析数据类型,返回字符子模式类型：
	*子模式类型: 1.ALPHA  2.LOWER   3.MIXED   4.PUNCTUATION
	*/
	int ParseData(unsigned char data, int idx, int length);

	/**
	*\brief
	*字节压缩模式: 通过基256到基900转换
	*/
	void Byte256To900(const unsigned char *str,int k);


	/**
	*数字压缩模式:从基10到基900转换
	*/
	void Number10To900(const unsigned char *str,int k, int size);


	/**
	*\brief
	*解析文本,并根据文本类型来插入填充、锁定码字
	*/
	void InsertFillCode();

	/**
	*\brief
	*当数据码字不够填充成一个矩形时,插入虚拟填充码字
	*/
	void InsertVirtualCode();

	/**
	*\brief
	*寻找错误等级数
	*/
	int MaxPossibleErrLevel(int m);

	/**
	*\brief
	*根据错误等级计算纠正码字,
	*/
	void CalculateErrorCorrection();

	/**
	*\brief
	*插入错误纠正码字
	*/
	void InsertErrCode();

	/**
	*\brief
	*计算左行、右行指示符,与起始符连接，计算公式如下:
	*		30xi + a, Ci = 0                   30xi + c, ci = 0
	*
	*Li =   30xi + b, Ci = 3            Ri  =  30xi + a, ci = 3
	*
	*		30xi + c, Ci = 6                   30xi + b, ci = 6 
	*
	*xi = int[(行号-1）/3], i =1,2,3,4,5 ...90。    Ci = 第 i 行族号。
	*a = int[(行数 - 1) / 3]。     b = 错误纠正等级数 * 3 + (行数 - 1) % 3;
	*c = 数据区的列数 - 1。
	*/
	void CulculateIndicate(int row, int colume);
	
	/**
	*\brief
	*插入左行和右行指示符
	*/
	void InsertIndicateCharat(int i,int value);

	
	/**
	*\brief
	*根据数据码字数量，计算行数和列数
	*/
	void CalaulateRowAndColumn();


	/**
	*\brief
	*对数据码字，纠错码字，填充码字，左右指示符，起始符，终止符，进行排列成一个矩阵
	*/
	void SortDataCodeWord();

	/**
	*\brief
	*获取起始符号的条空序列
	*/
	int getStartCharacters()const;

	/**
	*\brief
	*获取终止符号的条空序列,
	*/
	int getEndCharacters()const;

	/**
	*\biref
	*获取PDF417码字矩阵的条空序列
	*/
	std::vector<std::vector<int> > getBarSpace()const;

	/**
	*\brief
	*中文字节转换成码字
	*/
	void ByteToCodeWord(const unsigned char *str);

private:
	std::string m_Data;

	const unsigned char *m_RawData;

	PDF417MODE m_PDFMODE;

	//存储原始数据转换后的码字
	int m_Codeword[928];

	int codeptr;

	//码字长度
	int m_CodeWordLength;

	int m_Mode;

	//错误等级
	int m_level;

	//列数
	int m_Columns;

	//行数
	int m_Rows;

	std::vector<std::vector<int> > m_cluster;
};
