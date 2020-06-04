/**
	Prism Live: Code editor based on Prism.js
	Works best in Chrome. Currently only very basic support in other browsers (no snippets, no shortcuts)
	@author Lea Verou
*/
(async function() {

if (!window.Bliss) {
	// Load Bliss if not loaded
	console.log("Bliss not loaded. Loading remotely from blissfuljs.com");

	let bliss = document.createElement("script");
	bliss.src = "https://blissfuljs.com/bliss.shy.min.js";
	document.head.appendChild(bliss);

	await new Promise(resolve => bliss.onload = resolve);
}

var $ = Bliss, $$ = Bliss.$;
var ready = Promise.resolve();

if (document.currentScript) {
	// Tiny dynamic loader. Use e.g. ?load=css,markup,javascript to load components
	var base = document.currentScript.src;
	var load = new URL(base).searchParams.get("load");

	if (load !== null) {
		var files = ["../prism-live.css"];

		if (load) {
			files.push(...load.split(/,/).map(c => /\./.test(c)? c : `prism-live-${c}.js`));
		}

		ready = Promise.all(files.map(url => $.load(url, base)));
	}
}

var superKey = navigator.platform.indexOf("Mac") === 0? "metaKey" : "ctrlKey";

var _ = Prism.Live = class PrismLive {
	constructor(source) {
		this.source = source;
		this.sourceType = source.nodeName.toLowerCase();

		this.wrapper = $.create({
			className: "prism-live",
			around: this.source
		});

		if (this.sourceType === "textarea") {
			this.textarea = this.source;
			this.code = $.create("code");

			this.pre = $.create("pre", {
				className: this.textarea.className + " no-whitespace-normalization",
				contents: this.code,
				before: this.textarea
			});
		}
		else {
			this.pre = this.source;
			// this.pre.classList.add("no-whitespace-normalization");
			this.code = $("code", this.pre);

			this.textarea = $.create("textarea", {
				className: this.pre.className,
				value: this.pre.textContent,
				after: this.pre
			});
		}

		_.all.set(this.textarea, this);
		_.all.set(this.pre, this);
		_.all.set(this.code, this);

		this.pre.classList.add("prism-live");
		this.textarea.classList.add("prism-live");
		this.source.classList.add("prism-live-source");

		if (self.Incrementable) {
			// TODO data-* attribute for modifier
			// TODO load dynamically if not present
			new Incrementable(this.textarea);
		}

		$.bind(this.textarea, {
			input: evt => this.update(),

			keyup: evt => {
				if (evt.key == "Enter") { // Enter
					// Maintain indent on line breaks
					this.insert(this.currentIndent);
					this.syncScroll();
				}
			},

			keydown: evt => {
				if (evt.key == "Tab" && !evt.altKey) {
					// Default is to move focus off the textarea
					// this is never desirable in an editor
					evt.preventDefault();

					if (this.tabstops && this.tabstops.length > 0) {
						// We have tabstops to go
						this.moveCaret(this.tabstops.shift());
					}
					else if (this.hasSelection) {
						var before = this.beforeCaret("\n");
						var outdent = evt.shiftKey;

						this.selectionStart -= before.length;

						var selection = _.adjustIndentation(this.selection, {
							relative: true,
							indentation: outdent? -1 : 1
						});

						this.replace(selection);

						if (outdent) {
							var indentStart = _.regexp.gm`^${this.indent}`;
							var isBeforeIndented = indentStart.test(before);
							this.selectionStart += before.length + 1 - (outdent + isBeforeIndented);
						}
						else { // Indent
							var hasLineAbove = before.length == this.selectionStart;
							this.selectionStart += before.length + 1 + !hasLineAbove;
						}
					}
					else {
						// Nothing selected, expand snippet
						var selector = _.match(this.beforeCaret(), /\S*$/);
						var snippetExpanded = this.expandSnippet(selector);

						if (snippetExpanded) {
							requestAnimationFrame(() => $.fire(this.textarea, "input"));
						}
						else {
							this.insert(this.indent);
						}
					}
				}
				else if (_.pairs[evt.key]) {
					var other = _.pairs[evt.key];
					this.wrapSelection({
						before: evt.key,
						after: other,
						outside: true
					});
					evt.preventDefault();
				}
				else {
					for (let shortcut in _.shortcuts) {
						if (_.checkShortcut(shortcut, evt)) {
							_.shortcuts[shortcut].call(this, evt);
							evt.preventDefault();
						}
					}
				}
			},

			click: evt => {
				var l = this.getLine();
				var v = this.value;
				var ss = this.selectionStart;
				//console.log(ss, v[ss], l, v.slice(l.start, l.end));
			},

			"click keyup": evt => {
				if (!evt.key || evt.key.lastIndexOf("Arrow") > -1) {
					// Caret moved
					this.tabstops = null;
				}
			}
		});

		// this.syncScroll();
		this.textarea.addEventListener("scroll", this, {passive: true});

		$.bind(window, {
			"resize": evt => this.syncStyles()
		});

		// Copy styles with a delay
		requestAnimationFrame(() => {
			this.syncStyles();

			var sourceCS = getComputedStyle(this.source);

			this.pre.style.height = this.source.style.height || sourceCS.getPropertyValue("--height");
			this.pre.style.maxHeight = this.source.style.maxHeight || sourceCS.getPropertyValue("--max-height");
		});

		this.update();
		this.lang = (this.code.className.match(/lang(?:uage)?-(\w+)/i) || [,])[1];

		this.observer = new MutationObserver(r => {
			if (document.activeElement !== this.textarea) {
				this.textarea.value = this.pre.textContent;
			}
		});

		this.observe();

		this.source.dispatchEvent(new CustomEvent("prism-live-init", {bubbles: true, detail: this}));
	}

	handleEvent(evt) {
		if (evt.type === "scroll") {
			this.syncScroll();
		}
	}

	observe () {
		return this.observer && this.observer.observe(this.pre, {
			childList: true,
			subtree: true,
			characterData: true
		});
	}

	unobserve () {
		this.observer && this.observer.disconnect();
	}

	expandSnippet(text) {
		if (!text) {
			return false;
		}

		var context = this.context;

		if (text in context.snippets || text in _.snippets) {
			// Static Snippets
			var expansion = context.snippets[text] || _.snippets[text];
		}
		else if (context.snippets.custom) {
			var expansion = context.snippets.custom.call(this, text);
		}

		if (expansion) {
			// Insert snippet
			var stops = [];
			var replacement = [];
			var str = expansion;
			var match;

			while (match = _.CARET_INDICATOR.exec(str)) {
				stops.push(match.index + 1);
				replacement.push(str.slice(0, match.index + match[1].length));
				str = str.slice(match.index + match[0].length);
				_.CARET_INDICATOR.lastIndex = 0;
			}

			replacement.push(str);
			replacement = replacement.join("");

			if (stops.length > 0) {
				// make first stop relative to end, all others relative to previous stop
				stops[0] -= replacement.length;
			}

			this.delete(text);
			this.insert(replacement, {matchIndentation: true});
			this.tabstops = stops;
			this.moveCaret(this.tabstops.shift());
		}

		return !!expansion;
	}

	get selectionStart() {
		return this.textarea.selectionStart;
	}
	set selectionStart(v) {
		this.textarea.selectionStart = v;
	}

	get selectionEnd() {
		return this.textarea.selectionEnd;
	}
	set selectionEnd(v) {
		this.textarea.selectionEnd = v;
	}

	get hasSelection() {
		return this.selectionStart != this.selectionEnd;
	}

	get selection() {
		return this.value.slice(this.selectionStart, this.selectionEnd);
	}

	get value() {
		return this.textarea.value;
	}
	set value(v) {
		this.textarea.value = v;
	}

	get indent() {
		return _.match(this.value, /^[\t ]+/m, _.DEFAULT_INDENT);
	}

	get currentIndent() {
		var before = this.value.slice(0, this.selectionStart-1);
		return _.match(before, /^[\t ]*/mg, "", -1);
	}

	// Current language at caret position
	get currentLanguage() {
		var node = this.getNode();
		node = node? node.parentNode : this.code;
		var lang = _.match(node.closest('[class*="language-"]').className, /language-(\w+)/, 1);
		return _.aliases[lang] || lang;
	}

	// Get settings based on current language
	get context() {
		var lang = this.currentLanguage;
		return _.languages[lang] || _.languages.DEFAULT;
	}

	update () {
		var code = this.value;

		if (/\n$/.test(this.value)) {
			code += "\u200b";
		}

		this.unobserve();
		this.code.textContent = code;

		requestAnimationFrame(() => {
			this.observe();
		});

		Prism.highlightElement(this.code);
	}

	syncStyles() {
		// Copy pre metrics over to textarea
		var cs = getComputedStyle(this.pre);

		// Copy styles from <pre> to textarea
		this.textarea.style.caretColor = cs.color;

		var properties = /^(font|lineHeight)|[tT]abSize/gi;

		for (var prop in cs) {
			if (cs[prop] && prop in this.textarea.style && properties.test(prop)) {
				this.wrapper.style[prop] = cs[prop];
				this.textarea.style[prop] = this.pre.style[prop] = "inherit";
			}
		}

		this.update();
	}

	syncScroll() {
		if (this.pre.clientWidth === 0 && this.pre.clientHeight === 0) {
			return;
		}

		this.pre.scrollTop = this.textarea.scrollTop;
		this.pre.scrollLeft = this.textarea.scrollLeft;
	}

	beforeCaretIndex (until = "") {
		return this.value.lastIndexOf(until, this.selectionStart);
	}

	afterCaretIndex (until = "") {
		return this.value.indexOf(until, this.selectionEnd);
	}

	beforeCaret (until = "") {
		var index = this.beforeCaretIndex(until);

		if (index === -1 || !until) {
			index = 0;
		}

		return this.value.slice(index, this.selectionStart);
	}

	getLine(offset = this.selectionStart) {
		var value = this.value;
		var lf = "\n", cr = "\r";
		var start, end, char;

		for (var start = this.selectionStart; char = value[start]; start--) {
			if (char === lf || char === cr || !start) {
				break;
			}
		}

		for (var end = this.selectionStart; char = value[end]; end++) {
			if (char === lf || char === cr) {
				break;
			}
		}

		return {start, end};
	}

	afterCaret(until = "") {
		var index = this.afterCaretIndex(until);

		if (index === -1 || !until) {
			index = undefined;
		}

		return this.value.slice(this.selectionEnd, index);
	}

	setCaret(pos) {
		this.selectionStart = this.selectionEnd = pos;
	}

	moveCaret(chars) {
		if (chars) {
			this.setCaret(this.selectionEnd + chars);
		}
	}

	insert(text, {index} = {}) {
		if (!text) {
			return;
		}

		this.textarea.focus();

		if (index === undefined) {
			// No specified index, insert in current caret position
			this.replace(text);
		}
		else {
			// Specified index, first move caret there
			var start = this.selectionStart;
			var end = this.selectionEnd;

			this.selectionStart = this.selectionEnd = index;
			this.replace(text);

			this.selectionStart = start + (index < start? text.length : 0);
			this.selectionEnd = end + (index <= end? text.length : 0);
		}
	}

	// Replace currently selected text
	replace (text) {
		var hadSelection = this.hasSelection;

		this.insertText(text);

		if (hadSelection) {
			// By default inserText places the caret at the end, losing any selection
			// What we want instead is the replaced text to be selected
			this.selectionStart = this.selectionEnd - text.length;
		}
	}

	// Set text between indexes and restore caret position
	set (text, {start, end} = {}) {
		var ss = this.selectionStart;
		var se = this.selectionEnd;

		this.selectionStart = start;
		this.selectionEnd = end;

		this.insertText(text);

		this.selectionStart = ss;
		this.selectionEnd = se;
	}

	insertText (text) {
		if (!text) {
			return;
		}

		if (_.supportsExecCommand === true) {
			document.execCommand("insertText", false, text);
		}
		else if (_.supportsExecCommand === undefined) {
			// We still don't know if document.execCommand("insertText") is supported
			let value = this.value;

			document.execCommand("insertText", false, text);

			_.supportsExecCommand = value !== this.value;
		}

		if (_.supportsExecCommand === false) {
			this.textarea.setRangeText(text, this.selectionStart, this.selectionEnd, "end");
			requestAnimationFrame(() => this.update());
		}

		return _.supportsExecCommand;
	}

	/**
	 * Wrap text with strings
	 * @param before {String} The text to insert before
	 * @param after {String} The text to insert after
	 * @param start {Number} Character offset
	 * @param end {Number} Character offset
	 */
	wrap ({before, after, start = this.selectionStart, end = this.selectionEnd} = {}) {
		var ss = this.selectionStart;
		var se = this.selectionEnd;
		var between = this.value.slice(start, end);

		this.set(before + between + after, {start, end});

		if (ss > start) {
			ss += before.length;
		}

		if (se > start) {
			se += before.length;
		}

		if (ss > end) {
			ss += after.length;
		}

		if (se > end) {
			se += after.length;
		}

		this.selectionStart = ss;
		this.selectionEnd = se;
	}

	wrapSelection (o = {}) {
		var hadSelection = this.hasSelection;

		this.replace(o.before + this.selection + o.after);

		if (hadSelection) {
			if (o.outside) {
				// Do not include new text in selection
				this.selectionStart += o.before.length;
				this.selectionEnd -= o.after.length;
			}
		}
		else {
			this.moveCaret(-o.after.length);
		}
	}

	toggleComment() {
		var comments = this.context.comments;

		// Are we inside a comment?
		var node = this.getNode();
		var commentNode = node.parentNode.closest(".token.comment");

		if (commentNode) {
			// Remove comment
			var start = this.getOffset(commentNode);
			var commentText = commentNode.textContent;

			if (comments.singleline && commentText.indexOf(comments.singleline) === 0) {
				var end = start + commentText.length;
				this.set(this.value.slice(start + comments.singleline.length, end), {start, end});
				this.moveCaret(-comments.singleline.length);
			}
			else {
				comments = comments.multiline || comments;
				var end = start + commentText.length - comments[1].length;
				this.set(this.value.slice(start + comments[0].length, end), {start, end: end + comments[1].length});
			}
		}
		else {
			// Not inside comment, add
			if (this.hasSelection) {
				comments = comments.multiline || comments;

				this.wrapSelection({
					before: comments[0],
					after: comments[1]
				});
			}
			else {
				// No selection, wrap line
				// FIXME *inside indent*
				comments = comments.singleline? [comments.singleline, ""] : comments.multiline || comments;
				end = this.afterCaretIndex("\n");
				this.wrap({
					before: comments[0],
					after: comments[1],
					start: this.beforeCaretIndex("\n") + 1,
					end: end < 0? this.value.length : end
				});
			}
		}
	}

	duplicateContent() {
		var before = this.beforeCaret("\n");
		var after = this.afterCaret("\n");
		var text = before + this.selection + after;

		this.insert(text, {index: this.selectionStart - before.length});
	}

	delete(characters, {forward, pos} = {}) {
		var i = characters = characters > 0? characters : (characters + "").length;

		if (pos) {
			var selectionStart = this.selectionStart;
			this.selectionStart = pos;
			this.selectionEnd = pos + this.selectionEnd - selectionStart;
		}

		while (i--) {
			document.execCommand(forward? "forwardDelete" : "delete");
		}

		if (pos) {
			// Restore caret
			this.selectionStart = selectionStart - characters;
			this.selectionEnd = this.selectionEnd - pos + this.selectionStart;
		}
	}

	/**
	 * Get the text node at a given chracter offset
	 */
	getNode(offset = this.selectionStart, container = this.code) {
		var node, sum = 0;
		var walk = document.createTreeWalker(container, NodeFilter.SHOW_TEXT);

		while (node = walk.nextNode()) {
			sum += node.data.length;

			if (sum >= offset) {
				return node;
			}
		}

		// if here, offset is larger than maximum
		return null;
	}

	/**
	 * Get the character offset of a given node in the highlighted source
	 */
	getOffset(node) {
		var range = document.createRange();
		range.selectNodeContents(this.code);
		range.setEnd(node, 0);
		return range.toString().length;
	}

	// Utility method to get regex matches
	static match(str, regex, def, index = 0) {
		if (typeof def === "number" && arguments.length === 3) {
			index = def;
			def = undefined;
		}

		var match = str.match(regex);

		if (index < 0) {
			index = match.length + index;
		}

		return match? match[index] : def;
	}

	static checkShortcut(shortcut, evt) {
		return shortcut.trim().split(/\s*\+\s*/).every(key => {
			switch (key) {
				case "Cmd":   return evt[superKey];
				case "Ctrl":  return evt.ctrlKey;
				case "Shift": return evt.shiftKey;
				case "Alt":   return evt.altKey;
				default: return evt.key === key;
			}
		});
	}

	static registerLanguage(name, context, parent = _.languages.DEFAULT) {
		Object.setPrototypeOf(context, parent);
		return _.languages[name] = context;
	}

	static matchIndentation(text, currentIndent) {
		// FIXME this assumes that text has no indentation of its own
		// to make this more generally useful beyond snippets, we should first
		// strip text's own indentation.
		text = text.replace(/\r?\n/g, "$&" + currentIndent);
	}

	static adjustIndentation(text, {indentation, relative = true, indent = _.DEFAULT_INDENT}) {
		if (!relative) {
			// First strip min indentation
			var minIndent = text.match(_.regexp.gm`^(${indent})+`).sort()[0];

			if (minIndent) {
				text.replace(_.regexp.gm`^${minIndent}`, "");
			}
		}

		if (indentation < 0) {
			return text.replace(_.regexp.gm`^${indent}`, "");
		}
		else if (indentation > 0) { // Indent
			return text.replace(/^/gm, indent);
		}
	}
};

// Static properties
Object.assign(_, {
	all: new WeakMap(),
	ready,
	DEFAULT_INDENT: "\t",
	CARET_INDICATOR: /(^|[^\\])\$(\d+)/g,
	snippets: {
		"test": "Snippets work!",
	},
	pairs: {
		"(": ")",
		"[": "]",
		"{": "}",
		'"': '"',
		"'": "'",
		"`": "`"
	},
	shortcuts: {
		"Cmd + /": function() {
			this.toggleComment();
		},
		"Ctrl + Shift + D": function() {
			this.duplicateContent();
		}
	},
	languages: {
		DEFAULT: {
			comments: {
				multiline: ["/*", "*/"]
			},
			snippets: {}
		}
	},
	// Map of Prism language ids and their canonical name
	aliases: (() => {
		var ret = {};
		var canonical = new WeakMap(Object.entries(Prism.languages).map(x => x.reverse()).reverse());

		for (var id in Prism.languages) {
			var grammar = Prism.languages[id];

			if (typeof grammar !== "function") {
				ret[id] = canonical.get(grammar);
			}
		}

		return ret;
	})(),

	regexp: (() => {
		var escape = s => s.replace(/[-\/\\^$*+?.()|[\]{}]/g, "\\$&");
		var _regexp = (flags, strings, ...values) => {
			var pattern = strings[0] + values.map((v, i) => escape(v) + strings[i+1]).join("");
			return RegExp(pattern, flags);
		};
		var cache = {};

		return new Proxy(_regexp.bind(_, ""), {
			get: (t, property) => {
				return t[property] || cache[property]
					   || (cache[property] = _regexp.bind(_, property));
			}
		});
	})()
});

_.supportsExecCommand = document.execCommand? undefined : false;

$.ready().then(() => {
	$$(":not(.prism-live) > textarea.prism-live, :not(.prism-live) > pre.prism-live").forEach(source => {
		if (!_.all.get(source)) {
			new _(source);
		}
	});
});

})();
