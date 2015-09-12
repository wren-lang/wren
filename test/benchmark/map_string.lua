local adverbs = {
  "moderately", "really", "slightly", "very"
}

local adjectives = {
  "abandoned", "able", "absolute", "academic", "acceptable", "acclaimed",
  "accomplished", "accurate", "aching", "acidic", "acrobatic", "active",
  "actual", "adept", "admirable", "admired", "adolescent", "adorable", "adored",
  "advanced", "adventurous", "affectionate", "afraid", "aged", "aggravating",
  "aggressive", "agile", "agitated", "agonizing", "agreeable", "ajar",
  "alarmed", "alarming", "alert", "alienated", "alive", "all", "altruistic",
  "amazing", "ambitious", "ample", "amused", "amusing", "anchored", "ancient",
  "angelic", "angry", "anguished", "animated", "annual", "another", "antique",
  "anxious", "any", "apprehensive", "appropriate", "apt", "arctic", "arid",
  "aromatic", "artistic", "ashamed", "assured", "astonishing", "athletic",
  "attached", "attentive", "attractive", "austere", "authentic", "authorized",
  "automatic", "avaricious", "average", "aware", "awesome", "awful", "awkward",
  "babyish", "back", "bad", "baggy", "bare", "barren", "basic", "beautiful",
  "belated", "beloved", "beneficial", "best", "better", "bewitched", "big",
  "big-hearted", "biodegradable", "bite-sized", "bitter", "black",
  "black-and-white", "bland", "blank", "blaring", "bleak", "blind", "blissful",
  "blond", "blue", "blushing", "bogus", "boiling", "bold", "bony", "boring",
  "bossy", "both", "bouncy", "bountiful", "bowed", "brave", "breakable",
  "brief", "bright", "brilliant", "brisk", "broken", "bronze", "brown",
  "bruised", "bubbly", "bulky", "bumpy", "buoyant", "burdensome", "burly",
  "bustling", "busy", "buttery", "buzzing", "calculating", "calm", "candid",
  "canine", "capital", "carefree", "careful", "careless", "caring", "cautious",
  "cavernous", "celebrated", "charming", "cheap", "cheerful", "cheery", "chief",
  "chilly", "chubby", "circular", "classic", "clean", "clear", "clear-cut",
  "clever", "close", "closed", "cloudy", "clueless", "clumsy", "cluttered",
  "coarse", "cold", "colorful", "colorless", "colossal", "comfortable",
  "common", "compassionate", "competent", "complete", "complex", "complicated",
  "composed", "concerned", "concrete", "confused", "conscious", "considerate",
  "constant", "content", "conventional", "cooked", "cool", "cooperative",
  "coordinated", "corny", "corrupt", "costly", "courageous", "courteous",
  "crafty"
}

local animals = {
  "aardvark", "african buffalo", "albatross", "alligator", "alpaca", "ant",
  "anteater", "antelope", "ape", "armadillo", "baboon", "badger", "barracuda",
  "bat", "bear", "beaver", "bee", "bison", "black panther", "blue jay", "boar",
  "butterfly", "camel", "capybara", "carduelis", "caribou", "cassowary", "cat",
  "caterpillar", "cattle", "chamois", "cheetah", "chicken", "chimpanzee",
  "chinchilla", "chough", "clam", "cobra", "cockroach", "cod", "cormorant",
  "coyote", "crab", "crane", "crocodile", "crow", "curlew", "deer", "dinosaur",
  "dog", "dolphin", "domestic pig", "donkey", "dotterel", "dove", "dragonfly",
  "duck", "dugong", "dunlin", "eagle", "echidna", "eel", "elephant seal",
  "elephant", "elk", "emu", "falcon", "ferret", "finch", "fish", "flamingo",
  "fly", "fox", "frog", "gaur", "gazelle", "gerbil", "giant panda", "giraffe",
  "gnat", "goat", "goldfish", "goose", "gorilla", "goshawk", "grasshopper",
  "grouse", "guanaco", "guinea fowl", "guinea pig", "gull", "hamster", "hare",
  "hawk", "hedgehog", "heron", "herring", "hippopotamus", "hornet", "horse",
  "human", "hummingbird", "hyena", "ibex", "ibis", "jackal", "jaguar", "jay",
  "jellyfish", "kangaroo", "kingfisher", "koala", "komodo dragon", "kookabura",
  "kouprey", "kudu", "lapwing", "lark", "lemur", "leopard", "lion", "llama",
  "lobster", "locust", "loris", "louse", "lyrebird", "magpie", "mallard",
  "manatee", "mandrill", "mantis", "marten", "meerkat", "mink", "mole",
  "mongoose", "monkey", "moose", "mosquito", "mouse", "mule", "narwhal", "newt",
  "nightingale", "octopus", "okapi", "opossum", "oryx", "ostrich", "otter",
  "owl", "oyster", "parrot", "partridge", "peafowl", "pelican", "penguin",
  "pheasant", "pigeon", "pinniped", "polar bear", "pony", "porcupine",
  "porpoise", "prairie dog", "quail", "quelea", "quetzal", "rabbit", "raccoon",
  "ram", "rat", "raven", "red deer", "red panda", "reindeer", "rhinoceros",
  "rook", "salamander", "salmon", "sand dollar", "sandpiper", "sardine",
  "scorpion", "sea lion", "sea urchin", "seahorse", "shark", "sheep", "shrew",
  "skunk", "snail", "snake", "sparrow", "spider", "spoonbill", "squid",
  "wallaby", "wildebeest"
}

local keys = {}
for _, animal in ipairs(animals) do
  for _, adjective in ipairs(adjectives) do
    for _, adverb in ipairs(adverbs) do
      table.insert(keys, adverb .. " " .. adjective .. " " .. animal)
    end
  end
end

local start = os.clock()

local map = {}
local i = 0
for _, key in ipairs(keys) do
  map[key] = i
  i = i + 1
end

local sum = 0
for _, key in ipairs(keys) do
  sum = sum + map[key]
end

for _, key in ipairs(keys) do
  map[key] = nil
end

io.write(string.format("%d\n", sum))
io.write(string.format("elapsed: %.8f\n", os.clock() - start))
