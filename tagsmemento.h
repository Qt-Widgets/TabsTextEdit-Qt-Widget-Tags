#ifndef TAGSMEMENTO_H
#define TAGSMEMENTO_H
#include <QVector>
#include <QPen>
#include <QTimer>
#include <QtMath>
#include <QPainterPath>
#include <QPainter>
#include <QStyleHints>
#include <QGuiApplication>
#include <QTextLayout>
#include <QStyleOptionFrame>
#include <QtGui/private/qinputcontrol_p.h>

#include "tag.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define FONT_METRICS_WIDTH(fmt, ...) fmt.width(__VA_ARGS__)
#else
#define FONT_METRICS_WIDTH(fmt, ...) fmt.horizontalAdvance(__VA_ARGS__)
#endif

const int top_text_margin = 1;
const int bottom_text_margin = 1;
const int left_text_margin = 1;
const int right_text_margin = 1;

const int tag_horisontal_spacing = 3;
const int tag_inner_left_padding = 3;
const int tag_inner_right_padding = 4;
const int tag_inner_top_padding =3;
const int tag_inner_bottom_padding = 3;
const int tag_cross_width = 4;
const int tag_cross_spacing = 2;

class TagsPresenter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int m_currentEditIndex READ GetCurrentEditIndex)
public:
    TagsPresenter(QWidget *view, QTextLayout *m_editedTextLayout);
    ~TagsPresenter();
public://сетеры гетеры
    QVector<QString> GetTags() const;
    void SetTags(const QVector<QString> &newTags);
public://общие методы с коллекцией
    int GetTagsCount() const;
    void RemoveTagAtIndex(int index);
    void InsertEmptyTagAtIndex(int index);
    void EditNewTag();
public://курсор
    int GetCursorPosition() const;
    void SetCursorPosition(int position);
public:
    int GetCurrentEditIndex() const;
public:// текст на тэгах
    const QString& GetTagTextByIndex(int index);
    QString& UpdateTextInEdittedTag(const QString &text);
    QString& InsertCharectersInEdittedTagText(int startPositionInString, const QString& charecters);
    const QString& GetCurrentEdittedTagText() const;
    void RemoveCharecterInEditedTag();
private:
    QString& RemoveCharectersInEdittedTagText(int startPos, int count);
public://координаты
    void CalculateAllTagsRects();
    bool IsPointInCrossRectArea(int index, QPoint const& point) const;
    const QRect& GetTagRectByIndex(int index) const;
    const QRect GetEditedTranslatedTagRect()const;
    const QRect GetTranslatedTagRegByIndex(int index) const;
    const QRect&  GetCurrentEdittedTagRect() const;
    inline QRect GetCrossButtonRect(QRect const& r) const;
    int GetVericalScrollValue() const;
    void SetInputWidgetRect(const QRect &inputWidgetRect);
private:
    void CalculateTagsRects(QPoint& leftTopPoint, const QRect &widgetSizes, QVector<Tag> &Tags, int beginTagIndex, int lastTagIndex);
    void CalculateTagOnEdit(QPoint& leftTopPoint, const QRect &widgetSizes);
public:
    void CalculateVecticalScroll(QRect const& editedTagRect);
    void EditPreviousTag();
    void EditNextTag();
    void SetTagEditableAtIndex(int i);
    int GetAllTagsHeight() const;
private:


    void setEditingIndex(int index);


    QRect& SetCurrentEdittedTagRect(const QRect &rect);

public:

    int m_cursorBlinkTimerId;
    bool m_cursorBlinkStatus;





private:
    QWidget* m_guiWidget;
    QTextLayout *m_textLayout;
    QVector<Tag> m_tags;
    int m_vecticalScrollValue;
    int m_cursorPosition;
    int m_currentEditIndex;
    QRect m_inputWidgetRect;

};

#endif // TAGSMEMENTO_H
