include "CBarCodePDF417.h"

int main()
{
  CBarCodePDF417 bar("PDF417");
  
  bar.SortDataCodeWord();
  
  std::vector<vector<int>> vec = bar.getBarSpace();
 //qt  QPainter  draw PDF417
 /*
 QPainter painte;
 int Vspace = 0;
 for (size_t i = 0; i < vec.size(); i++)
		{
			int x = centerx;
			for (size_t j = 0; j < vec[i].size(); j++)
			{
				QString number = QString::number(vec[i][j]);
				for (int b = 0; b < number.length(); b++)
				{
					QString value = number.at(b);

					if (b % 2 == 0)           //条空序列有8位数字,索引从0开始，偶数位为条,奇数位为空
					{
						for (int k = 0; k < value.toInt(); k++)
						{
							painte.setPen(pen);
							painte.drawRect(x, Vspace, 2.0, 7.0);
							painte.fillRect(x, Vspace, 2.0, 7.0, QBrush(QColor(0, 0, 0)));
							x += 2.0;
							
						}
					}
					else
					{
						for (int k = 0; k < value.toInt(); k++)
							x +=  2.0;
					}
				}
			}
			Vspace += 7.0;
		}
 */
 
 
  retrurn 0 ;
 }
