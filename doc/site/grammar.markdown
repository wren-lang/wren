^title Grammar
^category reference

**TODO: Fill in the rest of the grammar.**

## Precedence

When you mix the different [method call][] syntaxes and other [control flow][]
operators together, you need to worry about *precedence*&mdash;which ones bind
more tightly than others&mdash;and *associativity*&mdash;how a series of the
same kind of call is ordered. Wren mostly follows C, except that it fixes [the
bitwise operator mistake][mistake]. The full precedence table, from tightest to
loosest, is:

[method call]: method-calls.html
[control flow]: control-flow.html
[mistake]: http://www.lysator.liu.se/c/dmr-on-or.html

<table class="precedence">
  <tbody>
    <tr>
      <th>Prec</th>
      <th>Operator</th>
      <th>Description</th>
      <th>Assoc</th>
    </tr>
    <tr>
      <td>1</td>
      <td><code>()</code> <code>[]</code> <code>.</code></td>
      <td>Grouping, Subscript, Method call</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>2</td>
      <td><code>-</code> <code>!</code> <code>~</code></td>
      <td>Negate, Not, Complement</td>
      <td>Right</td>
    </tr>
    <tr>
      <td>3</td>
      <td><code>*</code> <code>/</code> <code>%</code></td>
      <td>Multiply, Divide, Modulo</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>4</td>
      <td><code>+</code> <code>-</code></td>
      <td>Add, Subtract</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>5</td>
      <td><code>..</code> <code>...</code></td>
      <td>Inclusive range, Exclusive range</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>6</td>
      <td><code>&lt;&lt;</code> <code>&gt;&gt;</code></td>
      <td>Left shift, Right shift</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>7</td>
      <td><code>&lt;</code> <code>&lt;=</code> <code>&gt;</code> <code>&gt;=</code></td>
      <td>Comparison</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>8</td>
      <td><code>==</code></td>
      <td>Equals</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>8</td>
      <td><code>!=</code></td>
      <td>Not equal</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>9</td>
      <td><code>&amp;</code></td>
      <td>Bitwise and</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>10</td>
      <td><code>^</code></td>
      <td>Bitwise xor</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>11</td>
      <td><code>|</code></td>
      <td>Bitwise or</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>12</td>
      <td><code>is</code></td>
      <td>Type test</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>13</td>
      <td><code>&amp;&amp;</code></td>
      <td>Logical and</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>14</td>
      <td><code>||</code></td>
      <td>Logical or</td>
      <td>Left</td>
    </tr>
    <tr>
      <td>15</td>
      <td><code>?:</code></td>
      <td>Conditional</td>
      <td>Right</td>
    </tr>
    <tr>
      <td>16</td>
      <td><code>=</code></td>
      <td>Assign</td>
      <td>Right</td>
    </tr>
  </tbody>
</table>
