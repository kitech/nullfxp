
#include <QtCore>
#include <QtGui>

#include "rfsdirnode.h"


//////////////////////////////
///  RFSDirNode
/////////////////////////////

bool RFSDirNode::setMode(QString mode)
{
    this->mode = mode;
    //TODO 将字符串类型的mode转换成整数
    return true;
}
bool RFSDirNode::isDir()
{
    QChar ch = this->mode.at(0);
    if(ch == QChar('d') || ch == QChar('D') || ch == QChar('l')) {
        return true;
    }
    return false;
}
bool RFSDirNode::isSymlink()
{
    QChar ch = this->mode.at(0);
    if(ch == QChar('l')) {
        return true;
    }
    return false;
}

RFSDirNode * RFSDirNode::childAt(int row)
{
    if(row >= this->children.count()) return NULL;
    return this->children.at(row);
}

bool RFSDirNode::addChild(int row, RFSDirNode *node)
{
    if(row > this->children.count()) return false;
    node->rowNum = row;
    this->children.insert(row, node);
    RFSDirNode * n = 0;
    for(int i = row + 1 ; i < this->children.count() ; i++) {
        n = this->children.at(i);
        n->rowNum =+ 1;
    }
    return true;
}
bool RFSDirNode::removeChild(int row)
{
    if(row >= this->children.count()) return false;
    this->children.remove(row);
    RFSDirNode * n = 0;
    for(int i = row; i < this->children.count(); i++) {
        n = this->children.at(i);
        n->rowNum -= 1;
    }
    //TODO 清理删除掉的结点内在
    return true;
}
