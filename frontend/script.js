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
    updateTrieSVG(prefix, k);
});

/* ---------------- Editor Input ---------------- */

editor.addEventListener("input", () => {
    triggerAutocomplete();

    const prefix = getCurrentPrefix();
    updateTrieSVG(prefix, k);
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

  if (!match) {
    suggestions.innerHTML = ""; // 🔥 REQUIRED
    return;
  }

  const prefix = match[1];

  fetch(`http://localhost:8080/query?prefix=${prefix}&k=${k}`)
    .then(res => res.text())
    .then(data => {
      const words = data.split("\n").filter(Boolean);
      renderSuggestions(words);
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
    updateTrieSVG(prefix, k);
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



function fetchTrie() {
    fetch("http://localhost:8080/trie")
        .then(res => res.json())
        .then(data => renderTrie(data.root))
        .catch(() => {});
}

function fetchTrieJSON() {
    return fetch("http://localhost:8080/trie")
        .then(res => res.json())
        .then(data => data.root);
}
function traverseToPrefix(root, prefix) {
    let node = root;
    const path = [];

    for (const ch of prefix) {
        if (!node.children || !node.children[ch]) break;
        node = node.children[ch];
        path.push({ char: ch, node });
    }

    return { node, path };
}
function svgEl(tag, attrs = {}) {
    const el = document.createElementNS("http://www.w3.org/2000/svg", tag);
    for (const k in attrs) el.setAttribute(k, attrs[k]);
    return el;
}

function drawNode(svg, x, y, label, isEnd = false) {
    svg.appendChild(svgEl("circle", {
        cx: x,
        cy: y,
        r: 18,
        fill: isEnd ? "#c8e6c9" : "#e3f2fd",
        stroke: "#1565c0",
        "stroke-width": 2
    }));

    svg.appendChild(svgEl("text", {
        x,
        y: y + 5,
        "text-anchor": "middle",
        "font-size": "12"
    })).textContent = label;
}

function drawEdge(svg, x1, y1, x2, y2) {
    svg.appendChild(svgEl("line", {
        x1, y1, x2, y2,
        stroke: "#555",
        "stroke-width": 2
    }));
}

function renderTrie(node, prefix = "", depth = 0) {
    const container = document.getElementById("trie-visualiser");
    container.innerHTML = "<h3>Trie Visualiser</h3>";

    function dfs(n, word) {
        if (n.end) {
            const p = document.createElement("p");
            p.textContent = word + " (freq: " + n.freq + ")";
            container.appendChild(p);
        }

        for (const ch in n.children) {
            dfs(n.children[ch], word + ch);
        }
    }

    dfs(node, "");
}
function updateTrieSVG(prefix, k) {
    fetchTrieJSON().then(root => {
        const svg = document.getElementById("trie-svg");
        svg.innerHTML = "";

        const { node, path } = traverseToPrefix(root, prefix);

        const startX = 200;
        let y = 40;
        let prevX = startX;
        let prevY = y;

        // Draw root
        drawNode(svg, startX, y, "•");

        // Draw prefix path vertically
        path.forEach(p => {
            y += 80;
            drawEdge(svg, prevX, prevY, startX, y);
            drawNode(svg, startX, y, p.char, p.node.end);
            prevY = y;
        });

        // Draw top-K children horizontally
        if (node && node.children) {
            const children = Object.entries(node.children)
                .sort((a, b) => (b[1].freq || 0) - (a[1].freq || 0))
                .slice(0, k);

            const spacing = 70;
            const baseX = startX - ((children.length - 1) * spacing) / 2;
            const childY = y + 80;

            children.forEach(([ch, child], i) => {
                const x = baseX + i * spacing;
                drawEdge(svg, startX, y, x, childY);
                drawNode(svg, x, childY, ch, child.end);
            });
        }
    });
}
function drawTestTrie() {
    const svg = document.getElementById("trie-svg");
    svg.innerHTML = "";

    // Helper to create SVG elements
    function create(tag, attrs) {
        const el = document.createElementNS("http://www.w3.org/2000/svg", tag);
        for (const k in attrs) el.setAttribute(k, attrs[k]);
        return el;
    }

    // Example nodes
    const nodes = [
        { x: 200, y: 40, label: "root" },
        { x: 200, y: 120, label: "a" },
        { x: 150, y: 200, label: "p" },
        { x: 250, y: 200, label: "p" }
    ];

    const edges = [
        [0, 1],
        [1, 2],
        [1, 3]
    ];

    // Draw edges
    edges.forEach(([u, v]) => {
        svg.appendChild(create("line", {
            x1: nodes[u].x,
            y1: nodes[u].y,
            x2: nodes[v].x,
            y2: nodes[v].y,
            stroke: "#555",
            "stroke-width": 2
        }));
    });

    // Draw nodes
    nodes.forEach(n => {
        svg.appendChild(create("circle", {
            cx: n.x,
            cy: n.y,
            r: 18,
            fill: "#e3f2fd",
            stroke: "#1565c0",
            "stroke-width": 2
        }));

        svg.appendChild(create("text", {
            x: n.x,
            y: n.y + 5,
            "text-anchor": "middle",
            "font-size": "12",
            fill: "#000"
        })).textContent = n.label;
    });
}

function getCurrentPrefix() {
    const editor = document.getElementById("editor");
    const text = editor.value;
    const match = text.match(/([a-zA-Z]+)$/);
    return match ? match[1].toLowerCase() : "";
}

updateTrieSVG("", k);

//drawTestTrie();

//fetchTrie();
