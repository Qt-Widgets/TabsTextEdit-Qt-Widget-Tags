#include "tagslineeditwidget.h"

TagsLineEditWidget::TagsLineEditWidget(QWidget* parent)
    : QWidget(parent)

{
    QFont font=this->font();
    font.setPixelSize(14);
    this->setFont(font);
    m_tagsPresenter=new TagsPresenter(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);//Растягивание виджета
    setFocusPolicy(Qt::StrongFocus);//Фокус пропадает как мышь уходит
    setCursor(Qt::IBeamCursor);
    setMouseTracking(true);

    m_tagsPresenter->SetCursorVisible(hasFocus());
    m_tagsPresenter->UpdateDisplayText();
}

TagsLineEditWidget::~TagsLineEditWidget()
{

}

void TagsLineEditWidget::resizeEvent(QResizeEvent*)
{
    m_tagsPresenter->CalculateRects();
}

void TagsLineEditWidget::focusInEvent(QFocusEvent*)
{
    m_tagsPresenter->SetCursorVisible(true);
    m_tagsPresenter->UpdateDisplayText();
    m_tagsPresenter->CalculateRects();
    update();
}

void TagsLineEditWidget::focusOutEvent(QFocusEvent*)
{
    m_tagsPresenter->SetCursorVisible(false);
    m_tagsPresenter->UpdateDisplayText();
    m_tagsPresenter->CalculateRects();
    update();
}

void TagsLineEditWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    // opt
    QStyleOptionFrame panel;
    m_tagsPresenter->InitStyleOptionFrame(&panel);

    // draw frame
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &painter, this);//поле лайн эдита

    // clip
    QRect inputRect = m_tagsPresenter->GetInputWidgetRect();//поле где все + маргины
    painter.setClipRect(inputRect);//Включает обрезку и задает область клипа заданному прямоугольнику с помощью заданной операции клипа. По умолчанию используется операция замены текущего прямоугольника клипа.

    const QRect &editedTagRect = m_tagsPresenter->GetCurrentEdittedTagRect();
    const QPoint &editedTextPoint = editedTagRect.topLeft() + QPoint(tag_inner_left_padding,
                                                                  ((editedTagRect.height()/2 - fontMetrics().height()) / 2));

    // scroll
    m_tagsPresenter->CalculateVecticalScroll(editedTagRect);

    // tags
    m_tagsPresenter->DrawTags(painter, 0, m_tagsPresenter->currentEditIndex, 0, 0);

    // draw edited text
    auto const formatting = m_tagsPresenter->formatting();
    m_tagsPresenter->m_textLayout->draw(&painter, editedTextPoint - QPoint(0, m_tagsPresenter->m_vecticalScrollValue-top_text_margin-top_text_margin-bottom_text_margin-tag_inner_bottom_padding), formatting);
    // draw cursor
    if (m_tagsPresenter->m_cursorBlinkStatus)
    {
        m_tagsPresenter->m_textLayout->drawCursor(&painter, editedTextPoint - QPointF( 0, m_tagsPresenter->m_vecticalScrollValue-top_text_margin-top_text_margin-bottom_text_margin-tag_inner_bottom_padding), m_tagsPresenter->m_cursorPosition);
    }
    //end
    m_tagsPresenter->DrawTags(painter, m_tagsPresenter->currentEditIndex+1, m_tagsPresenter->tags.count(), 0, 0);
}

void TagsLineEditWidget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_tagsPresenter->m_cursorBlinkTimerId)
    {
        m_tagsPresenter->m_cursorBlinkStatus = !m_tagsPresenter->m_cursorBlinkStatus;
        update();
    }
}

void TagsLineEditWidget::mousePressEvent(QMouseEvent* event)
{
    bool hasTagFound = false;
    for (int i = 0; i < m_tagsPresenter->tags.count(); ++i)
    {
        if (m_tagsPresenter->IsPointInCrossRectArea(i, event->pos()))
        {
            m_tagsPresenter->tags.removeAt(i);
            if (i <= m_tagsPresenter->currentEditIndex)
            {
                m_tagsPresenter->currentEditIndex=m_tagsPresenter->currentEditIndex-1;
            }
            hasTagFound = true;
            break;
        }
        else
        {
            if (m_tagsPresenter->tags.at(i).rect.translated(0, -m_tagsPresenter->m_vecticalScrollValue).contains(event->pos()))
            {
                if (m_tagsPresenter->currentEditIndex == i)
                {
                    const QTextLine &textLine=m_tagsPresenter->m_textLayout->lineAt(0);
                    int xEditedTopLeft=m_tagsPresenter->GetCurrentEdittedTagRect().translated( 0, -m_tagsPresenter->m_vecticalScrollValue).topLeft().x();
                    int cursorPositionAtTheEndOfTag=textLine.xToCursor(event->pos().x() -xEditedTopLeft);
                    m_tagsPresenter->MoveCursor(cursorPositionAtTheEndOfTag, 0, false);
                }
                else
                {
                    m_tagsPresenter->SetTagEditableAtIndex(i);
                }

                hasTagFound = true;
                break;
            }
        }
    }

    if (!hasTagFound)
    {
        m_tagsPresenter->EditNewTag();
    }
    RepaintWidget();
}

QSize TagsLineEditWidget::sizeHint() const
{
    ensurePolished();
    QFontMetrics fontMetrics(font());
    int heightResult = fontMetrics.height() + 2 * vertical_margin + top_text_margin + bottom_text_margin + topmargin + bottommargin;
    int widthResult = fontMetrics.boundingRect(QLatin1Char('x')).width() * 17 + 2 * horizontal_margin + leftmargin + rightmargin; // "some"
    QStyleOptionFrame opt;
    m_tagsPresenter->InitStyleOptionFrame(&opt);
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt,
                                      QSize(widthResult, heightResult).expandedTo(QApplication::globalStrut()), this));
}

QSize TagsLineEditWidget::minimumSizeHint() const {
    ensurePolished();
    QFontMetrics fontmetrics = fontMetrics();
    int heightResult = fontmetrics.height() + qMax(2 * vertical_margin, fontmetrics.leading()) + top_text_margin + bottom_text_margin + topmargin + bottommargin;
    int widthResult = fontmetrics.maxWidth() + leftmargin + rightmargin;
    QStyleOptionFrame opt;
    m_tagsPresenter->InitStyleOptionFrame(&opt);
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt,
                                      QSize(widthResult, heightResult).expandedTo(QApplication::globalStrut()), this));
}

void TagsLineEditWidget::keyPressEvent(QKeyEvent* event)
{
    event->setAccepted(false);
    bool isCharecterKey = false;

    if (event == QKeySequence::SelectAll)
    {
        m_tagsPresenter->SelectAll();
        event->accept();
    }
    else
        if (event == QKeySequence::SelectPreviousChar)
        {
            m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->previousCursorPosition(m_tagsPresenter->m_cursorPosition), 0, true);
            event->accept();
        }
        else
            if (event == QKeySequence::SelectNextChar)
            {
                m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->nextCursorPosition(m_tagsPresenter->m_cursorPosition), 0, true);
                event->accept();
            }
            else
            {
                switch (event->key())
                {
                case Qt::Key_Left:
                {
                    if (m_tagsPresenter->m_cursorPosition == 0)
                    {
                        m_tagsPresenter->EditPreviousTag();
                    }
                    else
                    {
                        m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->previousCursorPosition(m_tagsPresenter->m_cursorPosition), 0, false);
                    }
                    event->accept();
                    break;
                }
                case Qt::Key_Right:
                {
                    if (m_tagsPresenter->m_cursorPosition == m_tagsPresenter->GetCurrentEdittedTagText().size())
                    {
                        m_tagsPresenter->EditNextTag();
                    }
                    else
                    {
                        m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->nextCursorPosition(m_tagsPresenter->m_cursorPosition), 0, false);
                    }
                    event->accept();
                    break;
                }
                case Qt::Key_Up:
                {
                    event->accept();
                    break;
                }
                case Qt::Key_Down:
                {
                    event->accept();
                    break;
                }
                case Qt::Key_Home:
                {
                    if (m_tagsPresenter->m_cursorPosition == 0)
                    {
                        m_tagsPresenter->SetTagEditableAtIndex(0);
                    }
                    else
                    {
                        m_tagsPresenter->MoveCursor(0, 0, false);
                    }
                    event->accept();
                    break;
                }
                case Qt::Key_End:
                {
                    if (m_tagsPresenter->m_cursorPosition == m_tagsPresenter->GetCurrentEdittedTagText().size())
                    {
                        m_tagsPresenter->SetTagEditableAtIndex(m_tagsPresenter->tags.size() - 1);
                    }
                    else
                    {
                        m_tagsPresenter->MoveCursor(m_tagsPresenter->GetCurrentEdittedTagText().length(), 0, false);
                    }
                    event->accept();
                    break;
                }
                case Qt::Key_Backspace:
                {
                    if (!m_tagsPresenter->GetCurrentEdittedTagText().isEmpty())
                    {
                        m_tagsPresenter->removeBackwardOne();
                    }
                    else
                    {
                        if (m_tagsPresenter->currentEditIndex > 0)
                        {
                            m_tagsPresenter->EditPreviousTag();
                        }
                    }
                    event->accept();
                    break;
                }
                case Qt::Key_Space :
                case Qt::Key_Enter :
                case Qt::Key_Return :
                {
                    if (!m_tagsPresenter->GetCurrentEdittedTagText().isEmpty())
                    {
                        m_tagsPresenter->tags.insert(m_tagsPresenter->tags.begin() + std::ptrdiff_t(m_tagsPresenter->currentEditIndex + 1), Tag());
                        m_tagsPresenter->EditNextTag();
                    }
                    event->accept();
                    break;
                }
                default:
                {
                    isCharecterKey = true;
                    break;
                }
                }
            }

    if (isCharecterKey && m_tagsPresenter->m_inputControl.isAcceptableInput(event))
    {
        if (m_tagsPresenter->hasSelection())
        {
            m_tagsPresenter->removeSelection();
        }
        m_tagsPresenter->InsertCharectersInEdittedTagText(m_tagsPresenter->m_cursorPosition, event->text());
        m_tagsPresenter->m_cursorPosition =m_tagsPresenter->m_cursorPosition + event->text().count();
        event->accept();
        isCharecterKey = false;
    }

    if (event->isAccepted())
    {
        RepaintWidget();
        Q_EMIT ToTagsEdited();
    }
}

void TagsLineEditWidget::SetTags(QVector<QString> const& tags)
{
    m_tagsPresenter->tags.clear();
    for (int i=0; i<tags.count(); ++i)
    {
        m_tagsPresenter->tags.append(Tag{tags.at(i), QRect()});
    }
    m_tagsPresenter->currentEditIndex = 0;
    m_tagsPresenter->MoveCursor(0, 0, false);

    m_tagsPresenter->EditNewTag();
    m_tagsPresenter->UpdateDisplayText();
    m_tagsPresenter->CalculateRects();

    update();
}

QVector<QString> TagsLineEditWidget::GetTags() const
{
    QVector<QString> tagList;
    for (Tag &item : m_tagsPresenter->tags)
    {
        tagList.append(item.text);
    }
    return tagList;
}

void TagsLineEditWidget::mouseMoveEvent(QMouseEvent* event)
{
    for (int i = 0; i < m_tagsPresenter->tags.size(); ++i)
    {
        if (m_tagsPresenter->IsPointInCrossRectArea(i, event->pos()))
        {
            setCursor(Qt::ArrowCursor);
            return;
        }
    }
    setCursor(Qt::IBeamCursor);
}

void TagsLineEditWidget::RepaintWidget()
{
    m_tagsPresenter->UpdateDisplayText();
    m_tagsPresenter->CalculateRects();
    m_tagsPresenter->updateCursorBlinking();
    update();
}

void TagsLineEditWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta();
    if(numDegrees>0)
    {
        if (m_tagsPresenter->m_cursorPosition == 0)
        {
            m_tagsPresenter->EditPreviousTag();
        }
        else
        {
            m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->previousCursorPosition(m_tagsPresenter->m_cursorPosition), 0, true);
        }
    }
    else
    {
        if (m_tagsPresenter->m_cursorPosition == m_tagsPresenter->GetCurrentEdittedTagText().size())
        {
            m_tagsPresenter->EditNextTag();
        }
        else
        {
            m_tagsPresenter->MoveCursor(m_tagsPresenter->m_textLayout->nextCursorPosition(m_tagsPresenter->m_cursorPosition), 0, true);
        }
    }
}
