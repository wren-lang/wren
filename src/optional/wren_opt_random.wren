foreign class Random {
  construct new() {
    seed_()
  }

  construct new(seed) {
    if (seed is Num) {
      seed_(seed)
    } else if (seed is Sequence) {
      if (seed.isEmpty) Fiber.abort("Sequence cannot be empty.")

      // TODO: Empty sequence.
      var seeds = []
      for (element in seed) {
        if (!(element is Num)) Fiber.abort("Sequence elements must all be numbers.")

        seeds.add(element)
        if (seeds.count == 16) break
      }

      // Cycle the values to fill in any missing slots.
      var i = 0
      while (seeds.count < 16) {
        seeds.add(seeds[i])
        i = i + 1
      }

      seed_(
          seeds[0], seeds[1], seeds[2], seeds[3],
          seeds[4], seeds[5], seeds[6], seeds[7],
          seeds[8], seeds[9], seeds[10], seeds[11],
          seeds[12], seeds[13], seeds[14], seeds[15])
    } else {
      Fiber.abort("Seed must be a number or a sequence of numbers.")
    }
  }

  foreign seed_()
  foreign seed_(seed)
  foreign seed_(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16)

  foreign float()
  float(end) { float() * end }
  float(start, end) { float() * (end - start) + start }

  foreign int()
  int(end) { (float() * end).floor }
  int(start, end) { (float() * (end - start)).floor + start }

  sample(list) { sample(list, 1)[0] }
  sample(list, count) {
    if (count > list.count) Fiber.abort("Not enough elements to sample.")

    // There at (at least) two simple algorithms for choosing a number of
    // samples from a list without replacement -- where we don't pick the same
    // element more than once.
    //
    // The first is faster when the number of samples is small relative to the
    // size of the collection. In many cases, it avoids scanning the entire
    // list. In the common case of just wanting one sample, it's a single
    // random index lookup.
    //
    // However, its performance degrades badly as the sample size increases.
    // Vitter's algorithm always scans the entire list, but it's also always
    // O(n).
    //
    // The cutoff point between the two follows a quadratic curve on the same
    // size. Based on some empirical testing, scaling that by 5 seems to fit
    // pretty closely and chooses the fastest one for the given sample and
    // collection size.
    if (count * count * 5 < list.count) {
      // Pick random elements and retry if you hit a previously chosen one.
      var picked = {}
      var result = []
      for (i in 0...count) {
        // Find an index that we haven't already selected.
        var index
        while (true) {
          index = int(count)
          if (!picked.containsKey(index)) break
        }

        picked[index] = true
        result.add(list[index])
      }

      return result
    } else {
      // Jeffrey Vitter's Algorithm R.

      // Fill the reservoir with the first elements in the list.
      var result = list[0...count]

      // We want to ensure the results are always in random order, so shuffle
      // them. In cases where the sample size is the entire collection, this
      // devolves to running Fisher-Yates on a copy of the list.
      shuffle(result)

      // Now walk the rest of the list. For each element, randomly consider
      // replacing one of the reservoir elements with it. The probability here
      // works out such that it does this uniformly.
      for (i in count...list.count) {
        var slot = int(0, i + 1)
        if (slot < count) result[slot] = list[i]
      }

      return result
    }
  }

  shuffle(list) {
    if (list.isEmpty) return

    // Fisher-Yates shuffle.
    for (i in 0...list.count - 1) {
      var from = int(i, list.count)
      var temp = list[from]
      list[from] = list[i]
      list[i] = temp
    }
  }
}
