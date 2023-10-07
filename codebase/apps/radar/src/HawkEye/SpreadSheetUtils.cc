
#include "SpreadSheetUtils.hh"

void SpreadSheetUtils::decode_pos(const QString &pos, int *row, int *col)
{
    if (pos.isEmpty()) {
        *col = -1;
        *row = -1;
    } else {
        *col = pos.at(0).toLatin1() - 'A';
        *row = pos.right(pos.size() - 1).toInt() - 1;
    }
}

QString SpreadSheetUtils::encode_pos(int row, int col)
{
  return QString::number(col + 'A') + QString::number(row + 1);
}
