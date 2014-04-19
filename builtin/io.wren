class IO {
  static print {
    IO.writeString_("\n")
  }

  static print(obj) {
    IO.writeString_(obj.toString)
    IO.writeString_("\n")
    return obj
  }

  static print(a1, a2) {
    printList_([a1, a2])
  }

  static print(a1, a2, a3) {
    printList_([a1, a2, a3])
  }

  static print(a1, a2, a3, a4) {
    printList_([a1, a2, a3, a4])
  }

  static print(a1, a2, a3, a4, a5) {
    printList_([a1, a2, a3, a4, a5])
  }

  static print(a1, a2, a3, a4, a5, a6) {
    printList_([a1, a2, a3, a4, a5, a6])
  }

  static print(a1, a2, a3, a4, a5, a6, a7) {
    printList_([a1, a2, a3, a4, a5, a6, a7])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {
    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16])
  }

  static printList_(objects) {
    for (object in objects) IO.writeString_(object.toString)
    IO.writeString_("\n")
  }

  static write(obj) {
    IO.writeString_(obj.toString)
    return obj
  }
}
