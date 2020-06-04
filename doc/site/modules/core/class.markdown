^title Class Class

**TODO**

## Methods

### **name**

The name of the class.

### **supertype**

The superclass of this class.

<pre class="snippet">
class Crustacean {}
class Crab is Crustacean {}

System.print(Crab.supertype) //> Crustacean
</pre>

A class with no explicit superclass implicitly inherits Object:

<pre class="snippet">
System.print(Crustacean.supertype) //> Object
</pre>

Object forms the root of the class hierarchy and has no supertype:

<pre class="snippet">
System.print(Object.supertype) //> null
</pre>