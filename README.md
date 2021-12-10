# TabsTextEdit-Qt-Widget

This is a simple, animated, high-performance input widget with textarea field.

Use https://github.com/nicktrandafil/tags if you need lineEdit field

Auto split input text into tags by comma, Space or Enter key.

Auto prevent duplicate tags. If you don't need this feature remove `m_tagsPresenter->IsEditedTextHasDuplicate()` in `keyPressEvent` method

How to use:

1. Copy TagsPresenter TagsLineEditWidget and Tag into your project
2. Add TagsLineEditWidget into layout or show it
3. Connect to signals if it nessessary

![1](.//images//1.gif)

UML Diagram:

![2](.//images//uml.png)
