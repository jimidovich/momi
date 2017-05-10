#ifndef RM_H
#define RM_H

#include <QObject>

class RM : public QObject {
	Q_OBJECT

public:
	RM(QObject *parent = nullptr);
	~RM();

private:
};

#endif // RM_H
