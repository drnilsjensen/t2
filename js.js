"use strict";

/* T2's Javascript handler */
/* two ugly globals to get geocode working and yellow pages up */
let csv = null;
let branchen = null;

/* handcrafted shebangs */
function shebang(httphead, url, tld) {
    if (url.includes("wiki")) {
	let words = prepare(url, "wikipedia", "wiki");
	url = "https://de.wikipedia.org/wiki/" + words[0] + "_" + words[1];
    } else if (url.includes("linkedin")) {
	let words = prepare(url, "linkedin", "");
	url = "https://de.linkedin.com/in/" + (words[0] + words[1]).toLowerCase();
    } else if (url.includes("kind")) {
	let words = prepare(url, "kinder", "kind");
	url = "https://www.fragfinn.de/?s=" + (words[0] + '+' + words[1]).toLowerCase();
    } else if (url.includes("twitt") || url.includes("mastodon")) {
	let words = prepare(url, "twitter", "mastodon");
	url = "https://mastodon.social/explore";
    } else if (url.includes("tube") || url.includes("video")) {
	let words = prepare(url, "youtube", "video");
	url = "https://peer.tube/search?search=" + (words[0] + ' ' + words[1]).toLowerCase() + "&searchTarget=search-index";
    } else if (yellow(url)) {
	let words = prepare(url, "gelbeseiten", "gelbe");
	if (isempty(words[1])) {
	    words[1] = "bundesweit";
	}
	url = "https://www.gelbeseiten.de/branchen/" + (words[0] + '/' + words[1]).toLowerCase();
    } else if (url == 'aa') {
	url = "https://www.anonyme-alkoholiker.de/";
    } else if (url == 'ai' || url == 'ki') {
	url = "https://chat.openai.com/";
    } else if (url == 'au') {
	url = "https://www.netdoktor.de/erste-hilfe/";
    } else if (url == 'ea') {
	url = "https://www.ea.com/de-de/";
    } else if (url == 'ei') {
	url = "https://www.eierfans.de/";
    } else if (url == 'eu') {
	url = "https://european-union.europa.eu/index_de/";
    } else if (url == 'ia') {
	// ;-)
	url = "https://www.arte.tv/de/search/?q=esel&genre=all";
    } else if (url == 'oo') {
	url = "https://www.wolframalpha.com/";
    } else if (url == 'oe' || url == 'at') {
	url = "https://www.moose.at/";
    } else if (url == 'ch') {
	url = "https://www.search.ch/";
    } else if (url == 'li') {
	url = "https://www.web.li/";
    } else if (url == 'io') {
	url = "https://www.computerbase.de/suche/?q=";
    } else if (url.includes('alternative')) {
	url = "https://www.alternative.to/";
    } else if (url == 'it') {
	url = "https://www.heise.de/suche/?q=";
    } else if (url == 'wetter') {
	url = "https://www.dwd.de/DE/Home/home_node.html";
    } else if (url == 'tv') {
	url = "https://www.tvspielfilm.de/";
    } else if (url == 'vc' || url == 'vk') {
	url = "https://www.senfcall.de/";
    } else if (url == 'xxx') {
	url = "https://weisser-ring.de/";
    } else {
	url = httphead + url + tld;
	// some people type in URLs into search engines...
	url = url.replace("www.www.", "www.");
	url = url.replace(tld + tld, tld);
	url = url.replace(/\++/g, "");
	url = url.replace(/ +/g, "-");
    }
    return url;
}

/* get all working */
function init() {
    if (sessionStorage) {
	p.value = sessionStorage.getItem("query");
	if (p && !isempty(p.value)) {
	    out.innerHTML = sessionStorage.getItem("out");
	}
    }
    if (localStorage) {
	let savedcode = localStorage.getItem("code");
	if (!savedcode) {
	    let dc = "{out.innerHTML = '&nbsp;<a href=\"javascript:glueck()\">Auf gut Glück!</a>';}";
	    localStorage.setItem("code", dc);
	    savedcode = dc;
	}
	eval(savedcode);
    }
    if (!csv) loadgeo();
    if (!branchen) loadbranchen();
}

function icat(s) {
    s = s.substring(1, s.length - 1);
    if (s[0] == '!') {
	s = s.substring(1);
	cat(s);
    }
    return eval(s);
}

function cat(s) {
    if (!localStorage) return;
    let dc = localStorage.getItem("code");
    if (!dc) dc = "";
    dc += ";{" + s + ";}";
    localStorage.setItem("code", dc);
}

/* the geo DB should be loaded once */
async function loadgeo() {
    if (csv) return;
    if (localStorage && !localStorage.getItem("geo")) {
	let response = await fetch("/plz_kfz.csv");
	let csv = await response.text();
	csv = csv.replace(/ [\n\r]/g, ',');
	localStorage.setItem("geo", csv);
    }
    if (localStorage) {
	csv = localStorage.getItem("geo");
	csv = csv.split(',');
    }
}

/* get text fragments to determine yellow pages mode */
async function loadbranchen() {
    if (branchen) return;
    let response = await fetch("/branchen.json");
    branchen = await response.json();
}

function isempty(s) {
    return !s || s.trim() === "";
}

/* capitalize first char */
function upperFirst(s) {
    return !s ? "" : s.charAt(0).toUpperCase() + s.substring(1);
}

/* remove input noise */
function prepare(url, t1, t2) {
    url = url.replace(t1, "");
    url = url.replace(t2, "");
    url = url.trim();
    url = url.replace(/\++/g, "");
    url = url.replace(/ +/g, " ");
    let words = url.split(" ");
    words[0] = upperFirst(words[0]);
    words[1] = upperFirst(words[1]);
    return words;
}

/* simplify German */
function prettify(s) {
    s = s.trim();
    s = s.toLowerCase();
    s = s.replace(/ä/g, "ae");
    s = s.replace(/ö/g, "oe");
    s = s.replace(/ü/g, "ue");
    s = s.replace(/ß/g, "ss");
    return s;
}

/* yellow page up for this search? */
function yellow(s) {
    let f = false;
    if (branchen) {
	branchen.forEach((i) => {f ||= s.includes(i);});
    }
    return f;
}

/* loved this button */
function glueck() {
    let httphead = "https://www.";
    let tld = ".de";
    if (p && !isempty(p.value)) {
	let url = p.value;
	url = shebang(httphead, url, tld);
	console.log(url);
	location.href = url;
    }
}

/* this replaces e.g. WF with 38xxx codes etc. */
function localize(s) {
    if (!csv) return s;
    let tmp = prepare(s, "", "");
    if (tmp[2]) {
	let x = csv.indexOf(tmp[2].toUpperCase());
	if (x > 0) {
	    tmp[2] = csv[x-1];
	    s = tmp[0] + ' ' + tmp[1];
	}
    } else if (tmp[1]) {
	let x = csv.indexOf(tmp[1].toUpperCase());
	if (x > 0) {
	    tmp[1] = csv[x-1];
	    s = tmp[0];
	}
    }
    // undo upper case from prepare
    s = s.toLowerCase();
    l.value = Number(tmp[2]);
    if (l.value == 'NaN' || l.value.length != 5) {
	l.value = Number(tmp[1]);
    }
    if (l.value == 'NaN' || l.value.length != 5) {
	l.value = 0;
    }
    return s;
}

/* sanitize and preprocess search query */
function validirium() {
    if (!p || isempty(p.value)) {
	return false;
    } else if (p.value[0] == '(') {
	out.innerHTML = icat(p.value);
	return false;
    } else {
	p.value = prettify(p.value);
	p.value = localize(p.value);
	if (localStorage && localStorage.getItem("code")) {
	    let code = localStorage.getItem("code");
	    eval(code);
	}
	if (sessionStorage) {
	    let pq = sessionStorage.getItem("query");
	    if (pq != p.value) {
		// reset search pointer
		r.value = 0;
		sessionStorage.setItem("query", p.value);
	    }
	    sessionStorage.setItem("out", out.innerHTML);
	}
	return true;
    }
}
