/**
 * @license
 * Based on jsmin.js by Franck Marcia. Modified by Bryan McQuade, to
 * make it match jsmin.c as of 2009/03/17. The original license is
 * below:
 *
 * jsmin.js - 2006-08-31
 * Author: Franck Marcia
 *
 * This work is an adaptation of jsminc.c published by Douglas Crockford.
 * Permission is hereby granted to use the Javascript version under the same
 * conditions as the jsmin.c on which it is based.
 *
 * jsmin.c
 * 2006-05-04
 *
 * Copyright (c) 2002 Douglas Crockford  (www.crockford.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * The Software shall be used for Good, not Evil.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

var JSMIN = {
  stringHas: function(str, c) {
    return str.indexOf(c) > -1;
  },

  compile: function(input) {

    var a = '',
        b = '',
        EOF = -1,
        LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz',
        DIGITS = '0123456789',
        ALNUM = LETTERS + DIGITS + '_$\\',
        theLookahead = EOF;


    /* isAlphanum -- return true if the character is a letter, digit, underscore,
            dollar sign, or non-ASCII character.
    */

    function isAlphanum(c) {
        return c != EOF && (JSMIN.stringHas(ALNUM, c) || c.charCodeAt(0) > 126);
    }


    /* get -- return the next character. Watch out for lookahead. If the
            character is a control character, translate it to a space or
            linefeed.
    */

    function get() {

        var c = theLookahead;
        theLookahead = EOF;
        if (c == EOF) {
            if (get.i < get.l) {
                c = input.charAt(get.i);
                ++get.i;
            }
        }
        if (c >= ' ' || c == '\n' || c == EOF) {
            return c;
        }
        if (c == '\r') {
            return '\n';
        }
        return ' ';
    }

    get.i = 0;
    get.l = input.length;


    /* peek -- get the next character without getting it.
    */

    function peek() {
        theLookahead = get();
        return theLookahead;
    }


    /* next -- get the next character, excluding comments. peek() is used to see
            if a '/' is followed by a '/' or '*'.
    */

    function next() {

        var c = get();
        if (c == '/') {
            switch (peek()) {
            case '/':
                for (;;) {
                    c = get();
                    if (c <= '\n') {
                        return c;
                    }
                }
            case '*':
                get();
                for (;;) {
                    switch (get()) {
                    case '*':
                        if (peek() == '/') {
                            get();
                            return ' ';
                        }
                        break;
                    case EOF:
                        throw 'Error: Unterminated comment.';
                    }
                }
            default:
                return c;
            }
        }
        return c;
    }


    /* action -- do something! What you do is determined by the argument:
            1   Output A. Copy B to A. Get the next B.
            2   Copy B to A. Get the next B. (Delete A).
            3   Get the next B. (Delete B).
       action treats a string as a single character. Wow!
       action recognizes a regular expression if it is preceded by ( or , or =.
    */

    function action(d) {

        var r = [];
        switch (d) {
        case 1:
            r.push(a);

        case 2:
            a = b;
            if (a == '\'' || a == '"') {
                for (;;) {
                    r.push(a);
                    a = get();
                    if (a == b) {
                        break;
                    }
                    if (a == '\\') {
                        r.push(a);
                        a = get();
                    }
                    if (a == EOF) {
                        throw 'Error: unterminated string literal: ' + a;
                    }
                }
            }

        case 3:
            b = next();

            if (b == '/' && JSMIN.stringHas('(,=:[!&|?{};\n', a)) {
                r.push(a);
                r.push(b);
                for (;;) {
                    a = get();
                    if (a == '/') {
                        break;
                    }
                    if (a =='\\') {
                        r.push(a);
                        a = get();
                    }
                    if (a == EOF) {
                        throw 'Error: unterminated Regular Expression literal';
                    }
                    r.push(a);
                }
                b = next();
            }
        }
        return r.join('');
    }


    /* m -- Copy the input to the output, deleting the characters which are
            insignificant to JavaScript. Comments will be removed. Tabs will be
            replaced with spaces. Carriage returns will be replaced with
            linefeeds.
            Most spaces and linefeeds will be removed.
    */

    function m() {

        var r = [];
        a = '\n';

        r.push(action(3));

        while (a != EOF) {
            switch (a) {
            case ' ':
                if (isAlphanum(b)) {
                    r.push(action(1));
                } else {
                    r.push(action(2));
                }
                break;
            case '\n':
                switch (b) {
                case '{':
                case '[':
                case '(':
                case '+':
                case '-':
                    r.push(action(1));
                    break;
                case ' ':
                    r.push(action(3));
                    break;
                default:
                    if (isAlphanum(b)) {
                        r.push(action(1));
                    } else {
                        r.push(action(2));
                    }
                }
                break;
            default:
                switch (b) {
                case ' ':
                    if (isAlphanum(a)) {
                        r.push(action(1));
                        break;
                    }
                    r.push(action(3));
                    break;
                case '\n':
                    switch (a) {
                    case '}':
                    case ']':
                    case ')':
                    case '+':
                    case '-':
                    case '"':
                    case '\'':
                        r.push(action(1));
                        break;
                    default:
                        if (isAlphanum(a)) {
                            r.push(action(1));
                        } else {
                            r.push(action(3));
                        }
                    }
                    break;
                default:
                    r.push(action(1));
                    break;
                }
            }
        }

        return r.join('');
    }

    return m();

}

};