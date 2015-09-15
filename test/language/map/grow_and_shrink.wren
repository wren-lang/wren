// This brute force test basically validates that the map can grow and shrink
// its capacity while still behaving correctly.

var fishes = [
  "Aeneus corydoras", "African glass catfish", "African lungfish",
  "Aholehole", "Airbreathing catfish", "Airsac catfish", "Alaska blackfish",
  "Albacore", "Alewife", "Alfonsino", "Algae eater", "Alligatorfish",
  "Alligator gar", "American sole", "Amur pike", "Anchovy", "Anemonefish",
  "Angelfish", "Angler", "Angler catfish", "Anglerfish", "Antarctic cod",
  "Antarctic icefish", "Antenna codlet", "Arapaima", "Archerfish",
  "Arctic char", "Armored gurnard", "Armored searobin", "Armorhead",
  "Armorhead catfish", "Armoured catfish", "Arowana", "Arrowtooth eel",
  "Aruana", "Asian carps", "Asiatic glassfish", "Atka mackerel",
  "Atlantic cod", "Atlantic eel", "Atlantic herring", "Atlantic salmon",
  "Atlantic saury", "Atlantic silverside", "Atlantic Trout",
  "Australasian salmon", "Australian grayling", "Australian herring",
  "Australian lungfish", "Australian prowfish", "Ayu", "Alooh",
  "Baikal oilfish", "Bala shark", "Ballan wrasse", "Bamboo shark",
  "Banded killifish", "Bandfish", "Banjo", "Bangus", "Banjo catfish", "Barb",
  "Barbel", "Barbeled dragonfish", "Barbeled houndshark", "Barblless catfish",
  "Barfish", "Barracuda", "Barracudina", "Barramundi", "Barred danio",
  "Barreleye", "Basking shark", "Bass", "Basslet", "Batfish", "Bat ray",
  "Beachsalmon", "Beaked salmon", "Beaked sandfish", "Beardfish",
  "Beluga sturgeon", "Bengal danio", "Bent tooth", "Betta", "Bichir",
  "Bicolor goat fish", "Bigeye", "Bigeye squaretail", "Bighead carp",
  "Bigmouth buffalo", "Bigscale", "Bigscale pomfret", "Billfish", "Bitterling",
  "Black angelfish", "Black bass", "Black dragonfish", "Blackchin",
  "Blackfish", "black neon tetra", "Blacktip reef shark", "Black mackerel",
  "Black pickerel", "Black prickleback", "Black scalyfin", "Black sea bass",
  "Black scabbardfish", "Blacksmelt", "Black swallower", "Black tetra",
  "Black triggerfish", "Bleak", "Blenny", "Blind goby", "Blind shark",
  "Blobfish", "Blowfish", "Blue catfish", "Blue danio", "Blue-redstripe danio",
  "Blue eye", "Bluefin tuna", "Bluefish", "Bluegill", "Blue gourami",
  "Blue shark", "Blue triggerfish", "Blue whiting", "Bluntnose knifefish",
  "Bluntnose minnow", "Boafish", "Boarfish", "Bobtail snipe eel", "Bocaccio",
  "Boga", "Bombay duck", "Bonefish", "Bonito", "Bonnetmouth", "Bonytail chub",
  "Bonytongue", "Bowfin", "Boxfish", "Bramble shark", "Bream", "Brill",
  "Bristlemouth", "Bristlenose catfish", "Broadband dogfish", "Brook lamprey",
  "Brook trout", "Brotula", "Brown trout", "Buffalo fish", "Bullhead",
  "Bullhead shark", "Bull shark", "Bull trout", "Burbot", "Bumblebee goby",
  "Buri", "Burma danio", "Burrowing goby", "Butterfly ray", "Butterflyfish",
  "California flyingfish", "California halibut", "California smoothtongue",
  "Canary rockfish", "Candiru", "Candlefish", "Capelin", "Cardinalfish",
  "Cardinal tetra", "Carp", "Carpetshark", "Carpsucker", "Catalufa", "Catfish",
  "Catla", "Cat shark", "Cavefish", "Celebes rainbowfish", "Central mudminnow",
  "Cepalin", "Chain pickerel", "Channel bass", "Channel catfish", "Char",
  "Cherry salmon", "Chimaera", "Chinook salmon", "Cherubfish", "Chub",
  "Chubsucker", "Chum salmon", "Cichlid", "Cisco", "Climbing catfish",
  "Climbing gourami", "Climbing perch", "Clingfish", "Clownfish",
  "Clown loach", "Clown triggerfish", "Cobbler", "Cobia", "Cod", "Cod icefish",
  "Codlet", "Codling", "Coelacanth", "Coffinfish", "Coho salmon", "Coley",
  "Collared carpetshark", "Collared dogfish", "Colorado squawfish", "Combfish",
  "Combtail gourami", "Combtooth blenny", "Common carp", "Common tunny",
  "Conger eel", "Convict blenny", "Convict cichlid", "Cookie-cutter shark",
  "Coolie loach", "Cornish Spaktailed Bream", "Cornetfish", "Cowfish",
  "Cownose ray", "Cow shark", "Crappie", "Creek chub", "Crestfish",
  "Crevice kelpfish", "Croaker", "Crocodile icefish", "Crocodile shark",
  "Crucian carp", "Cuchia", "Cuckoo wrasse", "Cusk-eel", "Cuskfish",
  "Cutlassfish", "Cutthroat eel", "Cutthroat trout"
]

var map = {}
for (fish in fishes) {
  map[fish] = fish.count
}

System.print(map.count) // expect: 249

for (n in 0...150) {
  map.remove(fishes[n])
}

System.print(map.count) // expect: 99

// Make sure we can still find all of the remaining ones.
var contained = 0
for (n in 150...249) {
  if (map.containsKey(fishes[n])) contained = contained + 1
}

System.print(contained) // expect: 99
