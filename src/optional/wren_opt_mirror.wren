
class Mirror {
  static reflect(reflectee) {
    var type = ObjectMirror.typeOf(reflectee)
    var mirror = ObjectMirror
    if (type.type == Class) mirror = ClassMirror
    if (type == Fiber) mirror = FiberMirror

    return mirror.reflect_(reflectee)
  }
}

class ObjectMirror is Mirror {
  foreign static canInvoke(reflectee, signature)
  foreign static typeOf(reflectee)

  static reflect_(reflectee) { new_(reflectee) }

  construct new_(reflectee) {
    _reflectee = reflectee
  }

  ==(rhs) { Object.same(reflectee, rhs.reflectee) }
  !=(rhs) { !(this == rhs) }

  classMirror { _classMirror ||
      (_classMirror = Mirror.reflect(ObjectMirror.typeOf(_reflectee))) }

  moduleMirror { classMirror.moduleMirror }

  reflectee { _reflectee }

  canInvoke(signature) { ObjectMirror.canInvoke(_reflectee, signature) }
}

class ClassMirror is ObjectMirror {
  foreign static hasMethod(reflectee, signature)
  foreign static methodNames(reflectee)
  foreign static module_(reflectee)

  static init_() {
    __cache = {}
  }

  static reflect_(reflectee) { __cache[reflectee] ||
      (__cache[reflectee] = ClassMirror.new_(reflectee)) }

  construct new_(reflectee) {
    super(reflectee)
  }

  moduleMirror { _moduleMirror ||
      (_moduleMirror = ModuleMirror.reflect_(ClassMirror.module_(reflectee))) }

  methodNames { _methodNames ||
      (_methodNames = ClassMirror.methodNames(reflectee)) }

  methodMirrors {
    if (_methodMirrors) return _methodMirrors

    _methodMirrors = {}
    for (methodName in methodNames) {
      _methodMirrors[methodName] = MethodMirror.new_(this, methodName, 0)
    }
    return _methodMirrors
  }

  hasMethod(signature) { ClassMirror.hasMethod(reflectee, signature) }
}
ClassMirror.init_()

class ClosureMirror is Mirror {
  foreign static boundToClass_(reflectee)
  foreign static module_(reflectee)
  foreign static signature_(reflectee)

  static reflect_(reflectee) { new_(reflectee) }

  construct new_(reflectee) {
    _moduleMirror = ModuleMirror.new_(ClosureMirror.module_(reflectee))
    _signature = ClosureMirror.signature_(reflectee)
  }

  reflectee_ { _reflectee }

  ==(rhs) { Object.same(_reflectee, rhs.reflectee_) }
  !=(rhs) { !(this == rhs) }

  moduleMirror { _moduleMirror }

  signature { _signature }
}

class FiberMirror is ObjectMirror {
  foreign static closureAt_(reflectee, stackTraceIndex)
  foreign static lineAt_(reflectee, stackTraceIndex)
  foreign static stackFramesCount_(reflectee)

  static reflect_(reflectee) { new_(reflectee) }

  construct new_(reflectee) {
    super(reflectee)
  }

  static current { reflect_(Fiber.current) }

  stackTrace() {
    var reflectee = this.reflectee

    return StackTrace.new_(reflectee, FiberMirror.stackFramesCount_(reflectee))
  }
}

class MethodMirror is Mirror {
  foreign static signature_(signatureIndex)

  construct new_(boundToClassMirror, signature, signatureIndex) {
    _boundToClassMirror = boundToClassMirror
    _signature = signature
    _signatureIndex = signatureIndex
  }

  boundToClassMirror { _boundToClassMirror }

  moduleMirror { _boundToClassMirror.moduleMirror }

//  arity { MethodMirror.arity_(_method) }
//  maxSlots { MethodMirror.maxSlots_(_method) }
//  numUpvalues { MethodMirror.maxSlots_(_numUpvalues) }
  signature { _signature }

  toString { _signature.toString }

  validate_(self, arity) {
    if (ObjectMirror.typeOf(self) != _boundToClassMirror.reflectee) Fiber.abort()
    //if (arity != this.arity) Fiber.abort()
    return self
  }

  call(self) {
    return validate_(self, 0).@"%(signature)"()
  }
  call(self, arg0) {
    return validate_(self, 1).@"%(signature)"(arg0)
  }
  call(self, arg0, arg1) {
    return validate_(self, 2).@"%(signature)"(arg0, arg1)
  }
  call(self, arg0, arg1, arg2) {
    return validate_(self, 3).@"%(signature)"(arg0, arg1, arg2)
  }
  call(self, arg0, arg1, arg2, arg3) {
    return validate_(self, 4).@"%(signature)"(arg0, arg1, arg2, arg3)
  }
  call(self, arg0, arg1, arg2, arg3, arg4) {
    return validate_(self, 5).@"%(signature)"(arg0, arg1, arg2, arg3, arg4)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5) {
    return validate_(self, 6).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6) {
    return validate_(self, 7).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) {
    return validate_(self, 8).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) {
    return validate_(self, 9).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) {
    return validate_(self, 10).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) {
    return validate_(self, 11).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) {
    return validate_(self, 12).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) {
    return validate_(self, 13).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) {
    return validate_(self, 14).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)
  }
  call(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) {
    return validate_(self, 15).@"%(signature)"(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14)
  }
}

class ModuleMirror is Mirror {
  foreign static current_
  foreign static fromName_(name)
  foreign static name_(reflectee)

  static init_() {
    __cache = {}
  }

  static reflect_(reflectee) { __cache[reflectee] ||
      (__cache[reflectee] = ModuleMirror.new_(reflectee)) }

  construct new_(reflectee) {
    _reflectee = reflectee
  }

  reflectee_ { _reflectee }

  ==(rhs) { Object.same(_reflectee, rhs.reflectee_) }
  !=(rhs) { !(this == rhs) }

  static current { reflect_(current_) }

  static fromName(name) {
    var module = fromName_(name)
    if (null == module) Fiber.abort("Unkown module")

    return reflect_(module)
  }

  name { ModuleMirror.name_(_reflectee) }
}
ModuleMirror.init_()

class StackTrace is Sequence {
  construct new_(fiber, stackFramesCount) {
    _fiber = fiber
    _stackTrace = []

    if (stackFramesCount == 0) return

    var stackFramesStart = stackFramesCount - (fiber != Fiber.current ? 1 : 2)

    for (stackFramesIndex in stackFramesStart..0) {
      _stackTrace.add(StackTraceFrame.new_(this, stackFramesIndex,
          ClosureMirror.new_(FiberMirror.closureAt_(fiber, stackFramesIndex)),
          FiberMirror.lineAt_(fiber, stackFramesIndex)))
    }
  }

  static new(fiber) { new_(fiber, FiberMirror.stackFramesCount_(fiber)) }

  count { _stackTrace.count }
  iterate(iterator) { _stackTrace.iterate(iterator) }
  iteratorValue(iterator) { _stackTrace.iteratorValue(iterator) }

  toString { _stackTrace.join("\n") }
}

class StackTraceFrame {
  construct new_(stackTrace, index, closureMirror, line) {
    _stackTrace = stackTrace
    _index = index

    _closureMirror = closureMirror
    _line = line
  }

  stackTrace { _stackTrace }
  index { _index }

  closureMirror { _closureMirror }
  line { _line }

  toString { "[%(_closureMirror.moduleMirror.name) line %(_line)] in %(_closureMirror.signature)" }
}
