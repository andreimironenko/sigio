.. posixcpptimer documentation master file, created by
   sphinx-quickstart on Wed Apr 24 15:19:01 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. .. header:: "section ###Section### ###SectNum###"
.. .. footer:: "Page ###Page###"

.. image:: tux.png
    :scale: 10%

Posix C++17 timer wrapper library documentation
===========================================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

:ref:`genindex`


=============================================================================
Docs
=============================================================================

Docs

------------
Sub-section
------------
*Italic Text*
This is a combination of normal text *with the italic* one.

^^^^^^^^^^^^^^^^^^
Sub-sub section
^^^^^^^^^^^^^^^^^^
Sub-sub text

""""""""""""""""""
Lists
""""""""""""""""""
**Bold Text**

* This is a bulleted list.
* It has two items, the second
  item uses two lines.

1. This is a numbered list.
2. It has two items too.



#. This is a numbered list.
#. It has two items too.

"""""""""""""""""""""""""""""
Nested list
"""""""""""""""""""""""""""""
* this is
* a list

  * with a nested list
  * and some subitems

* and here the parent list continues

Quoted paragraphs below
  Here a quote of Charichill "I am easily satisfied with the very best"


"""""""""""""""""""""""""""""
Line breaks
"""""""""""""""""""""""""""""
| These lines are
| broken exactly like in
| the source file


"""""""""""""""""""""""""""""
Code example
"""""""""""""""""""""""""""""

``Code example``

``Another code example``

.. code-block:: c++
  :linenos:

    explicit timer(std::chrono::duration<long, std::nano> period_nsec,
        callback_t callback = nullptr, void* data = nullptr,
        bool is_single_shot = false, int sig = SIGRTMAX
        );


Lorem ipsum [#f1]_ dolor sit amet ... [#f2]_

.. rubric:: Footnotes

.. [#f1] Text of the first footnote.
.. [#f2] Text of the second footnote.

+------------------------+------------+----------+----------+
| Header row, column 1   | Header 2   | Header 3 | Header 4 |
| (header rows optional) |            |          |          |
+========================+============+==========+==========+
| body row 1, column 1   | column 2   | column 3 | column 4 |
+------------------------+------------+----------+----------+
| body row 2             | ...        | ...      |          |
+------------------------+------------+----------+----------+

=====  =====  =======
A      B      A and B
=====  =====  =======
False  False  False
True   False  False
False  True   False
True   True   True
=====  =====  =======

=============================================================================
Class timer API
=============================================================================

.. doxygenclass:: posixcpp::timer
   :members:
   :private-members:
   :undoc-members:
