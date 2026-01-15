let DICTIONARY = [];

fetch("commondic.txt")
  .then(res => res.text())
  .then(text => {
    DICTIONARY = text
      .split("\n")
      .map(w => w.trim().toLowerCase())
      .filter(Boolean);

    console.log("Spell dictionary loaded:", DICTIONARY.length);
  });

const editor = document.getElementById("editor");
const suggestionsList = document.getElementById("suggestions");
const kSlider = document.getElementById("k-slider");
const kValue = document.getElementById("k-value");

let k = parseInt(kSlider.value, 10);
let activeIndex = -1;

kValue.textContent = k;


/* ---------------- Slider ---------------- */

kSlider.addEventListener("input", () => {
    k = parseInt(kSlider.value, 10);
    kValue.textContent = k;

    triggerAutocomplete();

    const prefix = getCurrentPrefix();
});

/* ---------------- Editor Input ---------------- */

editor.addEventListener("input", () => {
    triggerAutocomplete();

    const prefix = getCurrentPrefix();

});

/* ---------------- Keyboard Navigation ---------------- */

editor.addEventListener("keydown", (e) => {
    const items = suggestionsList.children;
    if (items.length === 0) return;

    if (e.key === "ArrowDown") {
        e.preventDefault();
        activeIndex = (activeIndex + 1) % items.length;
        updateActive(items);
    }

    if (e.key === "ArrowUp") {
        e.preventDefault();
        activeIndex = (activeIndex - 1 + items.length) % items.length;
        updateActive(items);
    }

    if (e.key === "Enter" && activeIndex >= 0) {
        e.preventDefault();
        insertSuggestion(items[activeIndex].textContent);
    }
});

/* ---------------- Core Logic ---------------- */

function triggerAutocomplete() {
  const text = editor.value;
  const match = text.match(/([a-zA-Z]+)$/);

  clearSuggestions();
  clearSpellSuggestions();

  if (!match) return;

  const prefix = match[1].toLowerCase();

  fetch(`http://localhost:8080/query?prefix=${prefix}&k=${k}`)
    .then(res => res.text())
    .then(data => {
      const words = data.split("\n").filter(Boolean);

     if (words.length > 0) {
  renderSuggestions(words);

  if (!DICTIONARY.includes(prefix)) {
    const spell = getSpellSuggestions(prefix);
    if (spell.length > 0) {
      renderSpellSuggestions(spell);
    }
  }
}
else {
        const spell = getSpellSuggestions(prefix);
        if (spell.length > 0) {
          renderSpellSuggestions(spell);
        }
      }
    });
}



function fetchSuggestions(prefix, k) {
    fetch(`http://localhost:8080/query?prefix=${prefix}&k=${k}`)
        .then(res => res.text())
        .then(data => {
            const words = data
                .split("\n")
                .map(w => w.trim())
                .filter(Boolean);

            renderSuggestions(words);
        })
        .catch(err => {
            console.error("Backend error:", err);
        });
}

function renderSuggestions(words) {
    clearSuggestions();
    activeIndex = -1;

    words.forEach(word => {
        const li = document.createElement("li");
        li.textContent = word;

        li.addEventListener("click", () => {
            insertSuggestion(word);
        });

        suggestionsList.appendChild(li);
    });
}

function insertSuggestion(word) {
    const text = editor.value;
    const match = text.match(/([a-zA-Z]+)$/);
    const prefix = match ? match[1] : "";

    fetch(`http://localhost:8080/select?prefix=${prefix}&word=${word}`)
        .catch(() => {});

    editor.value = text.replace(/([a-zA-Z]+)$/, word + " ");
    clearSuggestions();
}

/* ---------------- Helpers ---------------- */

function getCurrentPrefix(text) {
    const match = text.match(/([a-zA-Z]+)$/);
    return match ? match[1] : "";
}

function clearSuggestions() {
    suggestionsList.innerHTML = "";
}

function updateActive(items) {
    for (let i = 0; i < items.length; i++) {
        items[i].style.background = i === activeIndex ? "#1e293b" : "transparent";
    }
}

function isEditDistanceOne(a, b) {
  const la = a.length, lb = b.length;
  if (Math.abs(la - lb) > 1) return false;

  let i = 0, j = 0, diff = 0;

  while (i < la && j < lb) {
    if (a[i] === b[j]) {
      i++; j++;
    } else {
      diff++;
      if (diff > 1) return false;

      if (la > lb) i++;
      else if (lb > la) j++;
      else { i++; j++; }
    }
  }
  return true;
}

function getSpellSuggestions(word, limit = 3) {
  const results = [];
for (const w of DICTIONARY) {
  if (w[0] !== word[0]) continue;

  if (w.length < 3) continue;

  if (Math.abs(w.length - word.length) > 1) continue;

  if (isEditDistanceOne(word, w)) {
    results.push(w);
    if (results.length === limit) break;
  }
}

  return results;
}

const spellContainer = document.getElementById("spellcheck-container");
const spellList = document.getElementById("spell-suggestions");

function clearSpellSuggestions() {
  spellList.innerHTML = "";
  spellContainer.style.display = "none";
}

function renderSpellSuggestions(words) {
  clearSpellSuggestions();
  spellContainer.style.display = "block";

  words.forEach(word => {
    const li = document.createElement("li");
    li.textContent = word;
    li.onclick = () => replaceCurrentWord(word);
    spellList.appendChild(li);
  });
}

function replaceCurrentWord(word) {
  editor.value = editor.value.replace(/([a-zA-Z]+)$/, word + " ");
  clearSpellSuggestions();
}


function insertNewWord(word) {
    fetch(`http://localhost:8080/insert?word=${word}`)
        .then(() => console.log("Inserted:", word))
        .catch(() => {});
}

function handleInsert() {
    const input = document.getElementById("newWordInput");
    const word = input.value.trim();
    if (!word) return;

    insertNewWord(word);
    input.value = "";

    const prefix = getCurrentPrefix();
    //updateTrieSVG(prefix, k);
}
function handleDelete() {
  const input = document.getElementById("deleteWordInput");
  const word = input.value.trim();
  if (!word) return;

  fetch(`http://localhost:8080/delete?word=${word}`)
    .then(() => {
      console.log("Deleted:", word);
      triggerAutocomplete();
    })
    .catch(err => console.error(err));

  input.value = "";
}


function getCurrentPrefix() {
    const editor = document.getElementById("editor");
    const text = editor.value;
    const match = text.match(/([a-zA-Z]+)$/);
    return match ? match[1].toLowerCase() : "";
}



//drawTestTrie();

//fetchTrie();
