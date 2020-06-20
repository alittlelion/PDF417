#include "CBarCodePDF417.h"
#include <algorithm>
#include <string.h>

const int MAX_CODEWORD = 926;

/**
*\brief
*MIXED 模式字符
*/
const char *Mixed_Set = "0123456789&\r\t,:#-.$/+%*=^";

/**
*\brief
*PUNCTUATION 模式字符
*/
const char *Punctuation_Set = ";<>@[\\]_`~!\r\t,:\n-.$/\"|*()?{}'";


CBarCodePDF417::CBarCodePDF417()
{
}

CBarCodePDF417::CBarCodePDF417(std::string text)
{
	m_Data = text;
	m_RawData = reinterpret_cast<const unsigned char*>(m_Data.c_str());
	m_PDFMODE = TCMODE;
	memset(m_Codeword, 0, sizeof(m_Codeword));
	codeptr = 0;
	m_level = 0;
	m_CodeWordLength = 0;
	m_Rows = 3;
	m_Columns = 1;
	m_cluster.clear();
}


CBarCodePDF417::~CBarCodePDF417()
{

}

void CBarCodePDF417::setText(std::string text)
{
	m_Data = text;
	m_RawData = reinterpret_cast<const unsigned char*>(m_Data.c_str());
}

void CBarCodePDF417::setErrorLevel(int level)
{
	if(level <= 0 && level < 9)
		m_level = level;
}

void CBarCodePDF417::ByteCompress(const unsigned char *str, int length)
{
	int k = 0, j = 0;
	int size = (length / 6) * 5 + (length % 6);
	if (size + codeptr > MAX_CODEWORD)
	{
		return;
	}

	for (int k = 0; k < length; k++)
	{
		size = length - k < 6 ? length - k : 6;
		if (size < 6)
		{
			for (j = 0; j < size; j++)
			{
				m_Codeword[codeptr++] = (int)str[k + j] & 0xff;
			}
		}
		else
		{
			Byte256To900(str, k);
		}
	}

}

void CBarCodePDF417::Byte256To900(const unsigned char *str, int start)
{
	int length = 6;
	const unsigned char* text = str;
	int *ret = m_Codeword + codeptr;
	int retLast = 4;
	int i, nk;
	codeptr += retLast + 1;
	memset(ret, 0, (retLast + 1) * sizeof(int));
	length += start;
	for (i = start; i < length; i++)
	{
		for (nk = retLast; nk >= 0; nk--)
		{
			ret[nk] *= 256;
			int n = ret[nk];
		}
		ret[retLast] += (int)text[i] & 0xff;

		for (nk = retLast; nk > 0; nk--)
		{
			int n = ret[nk];
			ret[nk] %= 900;
			ret[nk - 1] += ret[nk] / 900;
		}
	}
}

void CBarCodePDF417::TextCompress(const unsigned char* text, int length)
{
	//空字符
	if (!text)
		return;

	int mode = ALPHA;
	int dest[5420] = { 0 };
	int ptr = 0;
	int v = 0;
	int fullByte = 0;
	for (int i = 0; i < length; i++)
	{
		char s = text[i];
		v = ParseData(s, i, length);
		if ((v &mode) != 0)
		{
			dest[ptr++] = v & 0xff;
			continue;
		}

		if ((v & ISBYTE) != 0)
		{
			//中文汉字，当有奇数值时，填充为偶数
			if ((ptr & 1) != 0)
			{
				//如果为 PUANTUATION 模式，则改变模式
				dest[ptr++] = (mode & PUNCTUATION) != 0 ? PAL : PS;
				mode = (mode & PUNCTUATION) != 0 ? ALPHA : mode;
			}
			dest[ptr++] = BYTESHIFT;
			dest[ptr++] = v & 0xff;
			fullByte += 2;
			continue;
		}

		switch (mode)
		{
		case ALPHA:
			//当前字符模式为 LOWER, 从 ALPHA -> LOWER
			if ((v & LOWER) != 0)
			{
				//记录ASCII码值，锁定LOWER模式
				dest[ptr++] = LL;
				dest[ptr++] = v & 0xff;
				mode = LOWER;
			}
			//当前字符模式为 MIXED，从 ALPHA -> MIXED
			else if ((v & MIXED) != 0)
			{
				//记录ASCII码值，锁定MIXED模式
				dest[ptr++] = ML;
				dest[ptr++] = v & 0xff;
				mode = MIXED;
			}
			//字符模式为 PUNCTUATION ，ALPHA -> PUNCTUATION
			else if ((ParseData(text[i + 1], i + 1, length) & ParseData(text[i + 2], i + 2, length) & PUNCTUATION) != 0)
			{
				dest[ptr++] = ML;
				dest[ptr++] = PL;
				dest[ptr++] = v & 0xff;
				mode = PUNCTUATION;
			}
			else
			{
				dest[ptr++] = PS;
				dest[ptr++] = v & 0xff;
			}
			continue;
		case LOWER:
			//当前字符模式为 ALPHA 字符
			if ((v & mode) != 0)
			{
				//后两个字符均为 ALPHA,从 LOWER->MIXED, MIXED->ALPHA. 并锁定模式为ALPHA 
				if ((ParseData(text[i + 1], i + 1, length) & ParseData(text[i + 2], i + 2, length) & ALPHA) != 0)
				{
					dest[ptr++] = ML;
					dest[ptr++] = AL;
					mode = ALPHA;
				}
				else
				{
					dest[ptr++] = AS;
				}
				dest[ptr++] = v & 0xff;
			}
			//当前字符模式为 MIXED 字符
			else if ((v & MIXED) != 0)
			{
				//记录ASCII码值，锁定模式为MIXED
				dest[ptr++] = ML;
				dest[ptr++] = v & 0xff;
				mode = MIXED;
			}
			//当前字符模式为 PUNCTUATION 
			else if ((ParseData(text[i + 1], i + 1, length) & ParseData(text[i + 2], i + 2, length) & PUNCTUATION) != 0)
			{
				//记录ASCII码值，锁定模式为PUNCTUATION
				dest[ptr++] = ML;
				dest[ptr++] = PL;
				dest[ptr++] = v & 0xff;
				mode = PUNCTUATION;
			}
			else
			{
				dest[ptr++] = PS;
				dest[ptr++] = v & 0xff;
			}
			continue;
		case MIXED:
			//当前字符模式为:LOWER
			if ((v & LOWER) != 0)
			{
				//MIXED -> LOWER,并记录ASCII码值，锁定模式为LOWER
				dest[ptr++] = LL;
				dest[ptr++] = v & 0xff;
				mode = LOWER;
			}
			//当前字符模式为:ALPHA
			else if ((v &ALPHA) != 0)
			{
				//MIXED -> ALPHA ,记录ASCII码值，锁定模式为ALPHA
				dest[ptr++] = AL;
				dest[ptr++] = v & 0xff;
				mode = ALPHA;
			}
			else if ((ParseData(text[i + 1], i + 1, length) & ParseData(text[i + 1], i + 2, length) & PUNCTUATION) != 0)
			{
				dest[ptr++] = PL;
				dest[ptr++] = v & 0xff;
				mode = PUNCTUATION;
			}
			else
			{
				dest[ptr++] = PS;
				dest[ptr++] = v & 0xff;
			}
			continue;
		case PUNCTUATION:
			dest[ptr++] = PAL;
			mode = ALPHA;
			i--;
			continue;
		default:
			break;
		}
	}
	if ((ptr & 1) != 0)
		dest[ptr++] = PS;

	int size = (ptr + fullByte) / 2;
	int len = ptr;
	ptr = 0;
	while (ptr < len)
	{
		v = dest[ptr++];
		if (v >= 30)
		{
			m_Codeword[codeptr++] = v;
			m_Codeword[codeptr++] = dest[ptr++];
		}
		else
		{
			m_Codeword[codeptr++] = v * 30 + dest[ptr++];
		}
	}
}


int CBarCodePDF417::ParseData(unsigned char ch, int idx, int length)
{
	if (idx > length)
		return 0;

	//大写字母子模式
	if (ch >= 'A' && ch <= 'Z')
		return (ALPHA + ch - 'A');

	//小写字母子模式
	if (ch >= 'a' && ch <= 'z')
		return  (LOWER + ch - 'a');

	//空格
	if (ch == ' ')
		return (ALPHA + LOWER + MIXED + SPACE);

	//MIXED 和 PUNCTUATION 字符是否存在
	const char *mix = strchr(Mixed_Set, ch);
	const char *pun = strchr(Punctuation_Set, ch);

	//汉字
	if (!mix && !pun)
		return (ISBYTE + (ch & 0xff));

	//既是 MIXED 字符和 PUNCTUATION 字符
	if (mix - Mixed_Set == pun - Punctuation_Set)
		return (MIXED + PUNCTUATION + (mix - Mixed_Set));

	//仅MIXED字符
	if (mix)
		return (MIXED + (mix - Mixed_Set));

	//仅PUNCTUATION字符
	return (PUNCTUATION + (pun - Punctuation_Set));
}



void CBarCodePDF417::NumberCompress(const unsigned char* str,int length)
{
	//int length = m_RawData.length();
	int full = (length / 44) * 15;
	int size = length % 44;

	int k;
	if (size == 0)
		size = full;
	else
		size = full + size / 3 + 1;

	if (size + codeptr > MAX_CODEWORD)
	{
		return;
	}

	for (k = 0; k < length; k += 44)
	{
		size = length - k < 44 ? length - k : 44;
		Number10To900(str, k, size);
	}
}

void CBarCodePDF417::Number10To900(const unsigned char *str, int start, int size)
{
	const unsigned char* text = str;

	int *ret = m_Codeword + codeptr;
	int retLast = size / 3;

	int i, k;
	codeptr += retLast + 1;
	memset(ret, 0, (retLast + 1) * sizeof(int));

	//加入前导符号1;
	ret[retLast] = 1;
	size += start;
	for (i = start; i < size; i++)
	{
		// (i * 10^i...0) * 10 
		for (k = retLast; k >= 0; k--)
		{
			ret[k] *= 10;
		}
		ret[retLast] += text[i] - '0';
		for (k = retLast; k > 0; k--)
		{
			ret[k - 1] += ret[k] / 900;
			ret[k] %= 900;
		}
	}

}


void CBarCodePDF417::InsertFillCode()
{
	if (!m_RawData)
		return;	

	size_t t = strlen((char*)m_RawData);
	
	//记录数字连续出现次数，及需要插入模式锁定位置
	int idx = 0;
	const unsigned char* numberStr;				 //连续为数字的字符串

	//记录字符出现次数，及需要插入模式锁定位置
	int charNumber = 0;
	const unsigned char* str;               //当连续数字小于13,存储的文本字符串
	codeptr = 1;
	std::vector<int> index;

	//记录中文字符
	std::string cnStr;
	std::vector<int> cnIndex;

	int byte = 0;
	for (int i = 0; i < strlen((char*)m_RawData); i++)
	{
		char ch = m_RawData[i];

		//记录连续数字
		if (ch >= '0' && ch <= '9')
		{
			idx++;
			if (idx >= 13)
			{
				numberStr += ch;
				charNumber = 0;
			}
			if (idx < 13)
			{
				numberStr += ch;
				index.push_back(i);
			}

			continue;
		}

		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '\r' || ch == '\t' || ch == '\n'|| ch == ' ')
		{
			charNumber++;
			str += ch;
			idx = 0;
		}

		if (ch & 0x80)
		{		
			byte++;		
			cnIndex.push_back(i);
			cnStr += ch;
			idx = 0;
		}

	}

	
	//如果第一个字符是文本字符,则启用 TCMODE 模式
	if ((m_RawData[0] >= ' ' || m_RawData[0] <= 127) && idx <13)
	{
		m_PDFMODE = TCMODE;
		const unsigned char* linkStr;
		if (idx < 13 && strlen((char*)m_RawData) < 13 && byte == 0)
		{
			TextCompress(m_RawData, strlen((char*)m_RawData));
		}
		
		else if (idx > 13 && byte == 0)
		{
			linkStr = str;
			TextCompress(linkStr, strlen((char*)linkStr));

			m_PDFMODE = NCMODE;
			m_Codeword[codeptr++] = m_PDFMODE;
			NumberCompress(numberStr, strlen((char*)numberStr));
		}

		else if (byte != 0)
		{
			//inkStr = m_RawData;
			//TextCompress(linkStr, linkStr.length());

			ByteToCodeWord(m_RawData);
		}
		else
		{		
			linkStr = m_RawData;
			TextCompress(linkStr, strlen((char*)linkStr));

		}
	}
	

	//如果数字连续字数大于13则启用数字模式
	else if (idx >= 13)
	{
		m_PDFMODE = NCMODE;
		m_Codeword[codeptr++] = m_PDFMODE;
		NumberCompress(numberStr,idx);

		//如果连续数字后面带英文字符，则插入模式锁定转移码字
		if (charNumber != 0 || byte != 0)
		{
			m_PDFMODE = TCMODE;
			m_Codeword[codeptr++] = m_PDFMODE;
			TextCompress(str, strlen((char*)str));
		}
	}		
	//Todo: 字节编码处理		
	else if (byte != 0 )
	{
		ByteToCodeWord(m_RawData);
	}
	
	//ByteToCodeWord(m_RawData);
	
	m_CodeWordLength = codeptr;
	m_Codeword[0] = codeptr;

	
}

void CBarCodePDF417::InsertVirtualCode()
{
	int divi = codeptr % m_Columns;
	if (divi != 0 && divi < m_Columns)
	{
		while (divi > 0) 
		{
			int position = codeptr;
			int value = 900;
			InsertIndicateCharat(position, value);
			m_CodeWordLength = codeptr;
			divi--;
		}
	}
}


int CBarCodePDF417::MaxPossibleErrLevel(int main)
{
	int size = 512;
	int level = 8;

	while (level > 0)
	{
		if (main >= size)
		{
			return level;
		}

		--level;
		size >>= 1;
	}

	return 0;
}

void CBarCodePDF417::CalculateErrorCorrection()
{
	if (m_level < 0 || m_level > 9)
		m_level = 0;

	//获取对应纠错表
	int *A = ERROR_LEVEL[m_level];

	//错误纠正容量,根据2(s+1)次方，求出纠错容量
	int Array = 2 << m_level;

	//将指针移到码字表尾
	int *dest = m_Codeword + m_CodeWordLength;

	int last = Array - 1;
	int t1 = 0, t2 = 0, t3 = 0;
	for (int i = 0; i < m_CodeWordLength; i++)
	{
		//多项式计算
		t1 = (m_Codeword[i] + dest[0]) % 929;
		for (int e = 0; e <= last; e++)
		{
			t2 = (t1 * A[last - e]) % 929;
			t3 = 929 - t2;
			int value = 0;
			value = ((e == last ? 0 : dest[e + 1]) + t3) % 929;
 			dest[e] = value;
		}
	}

	for (int j = 0; j < Array; j++)
	{
		int value = dest[j];
		dest[j] = (929 - dest[j]);	
		codeptr++;
	}
	m_CodeWordLength = codeptr;
}


void CBarCodePDF417::InsertErrCode()
{
	int ErrLevel = MaxPossibleErrLevel(928 - m_CodeWordLength);

	if (m_CodeWordLength < 41)
		m_level = 2;
	else if (m_CodeWordLength < 161)
		m_level = 3;
	else if (m_CodeWordLength < 321)
		m_level = 4;
	else
		m_level = 5;

	if (m_level < 0)
		m_level = 0;
	else if (m_level > ErrLevel)
		m_level = ErrLevel;

	CalculateErrorCorrection();
	
}


void CBarCodePDF417::CulculateIndicate(int row,int column)
{
	int tot = 0;
	//int *cluster ;
	int position = 0;

	for (int i = 0; i < row; i++)
	{
		int rowMod = i % 3;
		//cluster = PDF417CLUSTERS[rowMod];
		
		switch (rowMod)
		{
		case 0:
			tot = 30 * (i / 3) + ((row - 1) / 3);
			break;
		case 1:
			tot = 30 * (i / 3) + m_level * 3 + ((row - 1) % 3);
			break;
		default:			
			tot = 30 * (i / 3) + column - 1;
			break;
		}
		InsertIndicateCharat(position, tot);

		switch (rowMod)
		{
		case 0:
			tot = 30 * (i / 3) + column - 1;
			break;
		case 1:
			tot = 30 * (i / 3) + ((row - 1) / 3);
			break;
		default:
			tot = 30 * (i / 3) + m_level * 3 + (row - 1) % 3;
			break;
		}
		position += column + 1;
		InsertIndicateCharat(position, tot);
		position += 1;
	}

	m_CodeWordLength = codeptr;
	
	

	int iRow = codeptr / (column + 2);

	std::vector<int> col;
	int codewordCount = column + 2;
	int startPos = 0;
	for (int i = 0; i < iRow; i++)
	{
		
		for (int j = startPos; j < (startPos + codewordCount); j++)
		{
			col.push_back(m_Codeword[j]);
		}
		m_cluster.push_back(col);
		col.clear();
		startPos += codewordCount;
	}
	
}

void CBarCodePDF417::InsertIndicateCharat(int position,int value)
{
	int *dest = m_Codeword;

	for (int i = codeptr; i >= position; i--)
	{
		dest[i] = dest[i - 1];
		if (i == position)
		{
			dest[position] = value;
			codeptr++;
		}
	}

}


void CBarCodePDF417::SortDataCodeWord()
{
	if (!m_RawData)
		return;

	InsertFillCode();

	
	if (m_Rows < 3)
		m_Rows = 3;

	if (m_Columns < 1)
		m_Columns = 1;

	if (m_Rows > 90)
		m_Rows = 90;

	if (m_Columns > 30)
		m_Columns = 30;
	
	
	InsertErrCode();

	//先获取码字表，计算列数需要插入的虚拟填充字符数量
	CalaulateRowAndColumn();

	InsertVirtualCode();

	//再调用重新计算行数,计算左右行指示符
	CalaulateRowAndColumn();

	CulculateIndicate(m_Rows, m_Columns);

}


void CBarCodePDF417::CalaulateRowAndColumn()
{
	if (codeptr == 0)
		return;

	int row = 0;
	int column = 1;
	if (codeptr < 41 && codeptr > 0)
	{	
		row = codeptr / 2;
		column = 2;
		
	}
	else if (codeptr >= 41 )
	{		
		int a = 0;
		for (int i = 41; i < codeptr;)
		{
			a++;
			i += 40;
		}

		row = codeptr / (2 + a);
		column = a + 2;
	}	

	m_Rows = row;
	m_Columns = column;
}


int CBarCodePDF417::getStartCharacters()const
{
	//根据国际标准规定
	int start = 81111113;

	return start;
}

int CBarCodePDF417::getEndCharacters()const
{
	int end = 711311121;

	return end;
}


std::vector<std::vector<int> > CBarCodePDF417::getBarSpace()const
{
	//存储0 ， 3 ， 6 簇号条空码
	std::vector<std::vector<int> > barspace;
	std::vector<int> clusterRow;

	int start = getStartCharacters();
	int end = getEndCharacters();

	//数据码字列数加上 左右指示符列数
	int column = m_Columns + 2;
	int clusterCode = 0;
	for (int i = 0; i < m_cluster.size(); i++)
	{
		//加入起始符
		clusterRow.push_back(start);

		//第 i 行使用的簇号
		int clusterNumber = ((i) % 3) * 3;
		for (int j = 0; j < m_cluster[i].size(); j++)
		{


			if (clusterNumber == 0)   //0 簇号
				clusterCode = CLUSTER[0][m_cluster[i][j]];
			
			else if (clusterNumber == 3)   //3 簇号
				clusterCode = CLUSTER[1][m_cluster[i][j]];
			
			else       //6 簇号
				clusterCode = CLUSTER[2][m_cluster[i][j]];

			clusterRow.push_back(clusterCode);
		}

		//加入终止符
		clusterRow.push_back(end);

		barspace.push_back(clusterRow);
		clusterRow.clear();
	}

	return barspace;
}


void CBarCodePDF417::ByteToCodeWord(const unsigned char* str)
{
	if (!str)
		return;	

	int length = strlen((char*)str);
	if (length % 6 != 0)
		m_Codeword[codeptr++] = BCMODE;
	else
		m_Codeword[codeptr++] = BCMODE_6;
	
	std::vector<int> byteBits;
	int byte = 0;
	for (int i = 0; i < length; i++)
	{
		//将数据转换成字节
		char ch = str[i];

		if (ch & 0x80)
		{
			//byte = (ISBYTE + (ch & 0xff)) & 0xff;
			byte = ch & 0xff;
			byteBits.push_back(byte);			
		}
		else
		{
			byte = ch;
			byteBits.push_back(byte);
		}
	}	
	int bits;
	long long total = 0;
	int i= 0;
	std::vector<int> byteArray;
	
	/*
	for (int i = 0; i < byteBits.size(); i++)
	{
		int k = 5;
		k -= i;
		long long b = byteBits[i];
		while (k > 0)
		{
			b *= 256;
			k--;
		}
		total += b;
	}

	for (int L = 0; L < 5; L++)
	{
		int d = total % 900;
		total /= 900;
		byteArray.push_back(d);
	}
	std::reverse(byteArray.begin(), byteArray.end());
	for (auto a : byteArray)
	{
		m_Codeword[codeptr++] = a;
	}
	
	*/
	
	while (i < byteBits.size())
	{
		//字节数为6的倍数时
		if ( byteBits.size() % 6 == 0)
		{
			int div = byteBits.size() / 6;
			int q = 0;
			int j = 0;
			int len = 6;
			while (q < div)
			{
				int w = 0;
				for ( j ; j < len; j++)
				{
					int k = 5;
					
					k -= w;
					long long b = byteBits[j];
					while (k > 0)
					{
						b *= 256;
						k--;
					}
					total += b;
					w++;
				}
				len += 6;

				//基 256 到基 900 转换
				for (int L = 0; L < 5; L++)
				{
					int d = total % 900;
					total /= 900;
					byteArray.push_back(d);
				}
				std::reverse(byteArray.begin(), byteArray.end());
				for (auto a : byteArray)
				{
					m_Codeword[codeptr++] = a;
				}
				byteArray.clear();
				q++;
			}
		}
		else						 //字节数非 6 的倍数
		{
			int div = byteBits.size() / 6;
			int mod = byteBits.size() % 6;
			int s = div * 6 + mod;
			if (s < 6)				 //字节数小于 6
			{
				for (auto b : byteBits)
					m_Codeword[codeptr++] = b;
			}
			else					//字节数大于6
			{
				int q = 0;
				int j = 0;
				int len = 6;
				while (q < div)
				{
					int w = 0;
					for (j; j < len; j++)
					{
						int k = 5;

						k -= w;
						long long b = byteBits[j];
						while (k > 0)
						{
							//基 256 计算
							b *= 256;
							k--;
						}
						total += b;
						w++;
					}
					len += 6;

					//从基 256 到 基 900 转换
					for (int L = 0; L < 5; L++)
					{
						int d = total % 900;
						total /= 900;
						byteArray.push_back(d);
					}

					//反转数组，并压入码字表
					std::reverse(byteArray.begin(), byteArray.end());
					for (auto a : byteArray)
					{
						m_Codeword[codeptr++] = a;
					}
					byteArray.clear();
					q++;
				}

				for (int i = 0; i < mod; i++)
				{

					int b = byteBits[div * 6 + i];
					byteArray.push_back(b);
				}
				//std::reverse(byteArray.begin(), byteArray.end());
				for (auto a : byteArray)
				{
					m_Codeword[codeptr++] = a;
				}
				byteArray.clear();
				s -= 6;
			}
		}
		i += 6;
		break;
	}


	m_CodeWordLength = codeptr;

}
