// Libraries for Wi-Fi & webserver
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

// Setup mDNS: http://esp32.local
#include <ESPmDNS.h>

// DTH11 & DHT22 open-source library !installation required! (https://github.com/winlinvip/SimpleDHT)
#include <SimpleDHT.h>

// For Wake on LAN
#include <WiFiUdp.h>

// For JSON parsing
#include <ArduinoJson.h>

// Access point credentials
#include "secrets.h"

// For DHT11 module:
// Define the digital pin used to connect the module
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);

// Declare two byte variables for temperature and humidity
byte temperature = 0;
byte humidity = 0;

// Define the webserver on port 80
WebServer server(80);

// UDP instance for Wake on LAN
WiFiUDP udp;

// index.html
String page = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Ambient temperature and humidity</title>
  <meta name="description" content="Display ambient temperature and humidity">
  <link href="output.css" rel="stylesheet">
  <link rel="icon" href="favicon.svg">
</head>
<body class="flex flex-col items-center h-screen">
  <div class="text-center mt-10 md:mt-5 lg:mt-10">
    <button id="theme-toggle"
      class="p-2 bg-gray-200 rounded-full shadow-md hover:bg-gray-300 focus:outline-none dark:text-black">
      <svg xmlns="http://www.w3.org/2000/svg" width="1.5em" height="1.5em" viewBox="0 0 24 24" class="dark:hidden">
        <g fill="currentColor" fill-rule="evenodd" clip-rule="evenodd">
          <path d="M17 12a5 5 0 1 1-10 0a5 5 0 0 1 10 0m-1.5 0a3.5 3.5 0 1 1-7 0a3.5 3.5 0 0 1 7 0" />
          <path
            d="M7.455 3.464A1 1 0 0 1 9.017 2.8l2.337 1.64a1 1 0 0 0 1.164-.01l2.306-1.682a1 1 0 0 1 1.574.635l.492 2.812a1 1 0 0 0 .831.816l2.82.441a1 1 0 0 1 .664 1.563l-1.64 2.336a1 1 0 0 0 .01 1.164l1.683 2.307a1 1 0 0 1-.635 1.574l-2.812.492a1 1 0 0 0-.816.83l-.441 2.821a1 1 0 0 1-1.563.664l-2.336-1.64a1 1 0 0 0-1.164.01l-2.307 1.683a1 1 0 0 1-1.574-.636l-.492-2.812a1 1 0 0 0-.83-.816l-2.821-.44a1 1 0 0 1-.664-1.563l1.64-2.337a1 1 0 0 0-.01-1.164L2.75 9.182a1 1 0 0 1 .636-1.574l2.812-.493a1 1 0 0 0 .815-.83zm1.04 3.053l.317-2.028l1.68 1.18a2.5 2.5 0 0 0 2.91-.027l1.657-1.21l.354 2.022a2.5 2.5 0 0 0 2.076 2.039l2.027.317l-1.179 1.68a2.5 2.5 0 0 0 .027 2.91l1.209 1.657l-2.021.354a2.5 2.5 0 0 0-2.04 2.076l-.316 2.027l-1.68-1.179a2.5 2.5 0 0 0-2.91.026l-1.657 1.21l-.354-2.022a2.5 2.5 0 0 0-2.076-2.038l-2.027-.318l1.179-1.679a2.5 2.5 0 0 0-.027-2.91L4.435 8.947l2.021-.354a2.5 2.5 0 0 0 2.04-2.076" />
        </g>
      </svg>
      <svg xmlns="http://www.w3.org/2000/svg" width="1.5em" height="1.5em" viewBox="0 0 24 24"
        class="hidden dark:block">
        <path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
          d="M9 6a9 9 0 0 0 9 9c.91 0 1.787-.134 2.614-.385A9 9 0 0 1 12 21A9 9 0 0 1 9.386 3.386A9 9 0 0 0 9 6" />
      </svg>
    </button>
  </div>
  <div class="flex flex-row container justify-around">
    <div>
      <h1 id="t-heading" class="text-2xl font-semibold text-center">Temperature :</h1>
      <div class="relative flex items-center justify-center aspect-square">
        <svg class="transform -rotate-90 w-full h-full">
          <circle cx="50%" cy="50%" r="90" fill="none" stroke="#e5e7eb" stroke-width="12" />
          <circle id="t" cx="50%" cy="50%" r="90" fill="none" stroke="#4caf50" stroke-width="12" stroke-linecap="round"
            stroke-dasharray="565.48" stroke-dashoffset="565.48" style="transition: stroke-dashoffset 1s ease;" />
        </svg>
        <div class="absolute text-2xl font-bold text-center text-gray-700 dark:text-gray-100" id="t-text">0&#37;</div>
      </div>
    </div>
    <div>
      <h1 id="h-heading" class="text-2xl font-semibold text-center">Humidity :</h1>
      <div class="relative flex items-center justify-center aspect-square">
        <svg class="transform -rotate-90 w-full h-full">
          <circle cx="50%" cy="50%" r="90" fill="none" stroke="#e5e7eb" stroke-width="12" />
          <circle id="h" cx="50%" cy="50%" r="90" fill="none" stroke="#4caf50" stroke-width="12" stroke-linecap="round"
            stroke-dasharray="565.48" stroke-dashoffset="565.48" style="transition: stroke-dashoffset 1s ease;" />
        </svg>
        <div class="absolute text-2xl font-bold text-center text-gray-700 dark:text-gray-100" id="h-text">0&#37;</div>
      </div>
    </div>
  </div>
  <div class="absolute bottom-0 mb-5 text-gray-400">
    <p>Status: <span id="status"></span></p>
  </div>
  <script src="script.js"></script>
</body>
</html>
)";

// Serve the index.html
void htmlIndex() {
  server.send(200, "text/html", page);
}

// favicon.svg
String favicon = R"(
  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
    <path fill="#9ca3af" d="M19 8a2 2 0 0 1 2 2v6.76c.61.55 1 1.35 1 2.24c0 1.66-1.34 3-3 3s-3-1.34-3-3c0-.89.39-1.69 1-2.24V10c0-1.1.9-2 2-2m0 1c-.55 0-1 .45-1 1v1h2v-1c0-.55-.45-1-1-1M5 20v-8H2l10-9l4.4 3.96A3.97 3.97 0 0 0 15 10v6c-.63.83-1 1.87-1 3l.1 1z"/>
  </svg>
)";

// Serve the favicon.svg
void htmlFavicon() {
  server.send(200, "image/svg+xml", favicon);
}

// output.css
String css = R"(*, ::before, ::after {--tw-border-spacing-x: 0;--tw-border-spacing-y: 0;--tw-translate-x: 0;--tw-translate-y: 0;--tw-rotate: 0;--tw-skew-x: 0;--tw-skew-y: 0;--tw-scale-x: 1;--tw-scale-y: 1;--tw-pan-x: ;--tw-pan-y: ;--tw-pinch-zoom: ;--tw-scroll-snap-strictness: proximity;--tw-gradient-from-position: ;--tw-gradient-via-position: ;--tw-gradient-to-position: ;--tw-ordinal: ;--tw-slashed-zero: ;--tw-numeric-figure: ;--tw-numeric-spacing: ;--tw-numeric-fraction: ;--tw-ring-inset: ;--tw-ring-offset-width: 0px;--tw-ring-offset-color: #fff;--tw-ring-color: rgb(59 130 246 / 0.5);--tw-ring-offset-shadow: 0 0 #0000;--tw-ring-shadow: 0 0 #0000;--tw-shadow: 0 0 #0000;--tw-shadow-colored: 0 0 #0000;--tw-blur: ;--tw-brightness: ;--tw-contrast: ;--tw-grayscale: ;--tw-hue-rotate: ;--tw-invert: ;--tw-saturate: ;--tw-sepia: ;--tw-drop-shadow: ;--tw-backdrop-blur: ;--tw-backdrop-brightness: ;--tw-backdrop-contrast: ;--tw-backdrop-grayscale: ;--tw-backdrop-hue-rotate: ;--tw-backdrop-invert: ;--tw-backdrop-opacity: ;--tw-backdrop-saturate: ;--tw-backdrop-sepia: ;--tw-contain-size: ;--tw-contain-layout: ;--tw-contain-paint: ;--tw-contain-style: ;}::backdrop {--tw-border-spacing-x: 0;--tw-border-spacing-y: 0;--tw-translate-x: 0;--tw-translate-y: 0;--tw-rotate: 0;--tw-skew-x: 0;--tw-skew-y: 0;--tw-scale-x: 1;--tw-scale-y: 1;--tw-pan-x: ;--tw-pan-y: ;--tw-pinch-zoom: ;--tw-scroll-snap-strictness: proximity;--tw-gradient-from-position: ;--tw-gradient-via-position: ;--tw-gradient-to-position: ;--tw-ordinal: ;--tw-slashed-zero: ;--tw-numeric-figure: ;--tw-numeric-spacing: ;--tw-numeric-fraction: ;--tw-ring-inset: ;--tw-ring-offset-width: 0px;--tw-ring-offset-color: #fff;--tw-ring-color: rgb(59 130 246 / 0.5);--tw-ring-offset-shadow: 0 0 #0000;--tw-ring-shadow: 0 0 #0000;--tw-shadow: 0 0 #0000;--tw-shadow-colored: 0 0 #0000;--tw-blur: ;--tw-brightness: ;--tw-contrast: ;--tw-grayscale: ;--tw-hue-rotate: ;--tw-invert: ;--tw-saturate: ;--tw-sepia: ;--tw-drop-shadow: ;--tw-backdrop-blur: ;--tw-backdrop-brightness: ;--tw-backdrop-contrast: ;--tw-backdrop-grayscale: ;--tw-backdrop-hue-rotate: ;--tw-backdrop-invert: ;--tw-backdrop-opacity: ;--tw-backdrop-saturate: ;--tw-backdrop-sepia: ;--tw-contain-size: ;--tw-contain-layout: ;--tw-contain-paint: ;--tw-contain-style: ;}*, ::before, ::after {box-sizing: border-box;border-width: 0;border-style: solid;border-color: #e5e7eb;}::before, ::after {--tw-content: '';}html, :host {line-height: 1.5;-webkit-text-size-adjust: 100%;-moz-tab-size: 4;-o-tab-size: 4;tab-size: 4;font-family: ui-sans-serif, system-ui, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji";font-feature-settings: normal;font-variation-settings: normal;-webkit-tap-highlight-color: transparent;}body {margin: 0;line-height: inherit;}hr {height: 0;color: inherit;border-top-width: 1px;}abbr:where([title]) {-webkit-text-decoration: underline dotted;text-decoration: underline dotted;}h1, h2, h3, h4, h5, h6 {font-size: inherit;font-weight: inherit;}a {color: inherit;text-decoration: inherit;}b, strong {font-weight: bolder;}code, kbd, samp, pre {font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;font-feature-settings: normal;font-variation-settings: normal;font-size: 1em;}small {font-size: 80%;}sub, sup {font-size: 75%;line-height: 0;position: relative;vertical-align: baseline;}sub {bottom: -0.25em;}sup {top: -0.5em;}table {text-indent: 0;border-color: inherit;border-collapse: collapse;}button, input, optgroup, select, textarea {font-family: inherit;font-feature-settings: inherit;font-variation-settings: inherit;font-size: 100%;font-weight: inherit;line-height: inherit;letter-spacing: inherit;color: inherit;margin: 0;padding: 0;}button, select {text-transform: none;}button, input:where([type='button']), input:where([type='reset']), input:where([type='submit']) {-webkit-appearance: button;background-color: transparent;background-image: none;}:-moz-focusring {outline: auto;}:-moz-ui-invalid {box-shadow: none;}progress {vertical-align: baseline;}::-webkit-inner-spin-button, ::-webkit-outer-spin-button {height: auto;}[type='search'] {-webkit-appearance: textfield;outline-offset: -2px;}::-webkit-search-decoration {-webkit-appearance: none;}::-webkit-file-upload-button {-webkit-appearance: button;font: inherit;}summary {display: list-item;}blockquote, dl, dd, h1, h2, h3, h4, h5, h6, hr, figure, p, pre {margin: 0;}fieldset {margin: 0;padding: 0;}legend {padding: 0;}ol, ul, menu {list-style: none;margin: 0;padding: 0;}dialog {padding: 0;}textarea {resize: vertical;}input::-moz-placeholder, textarea::-moz-placeholder {opacity: 1;color: #9ca3af;}input::placeholder, textarea::placeholder {opacity: 1;color: #9ca3af;}button, [role="button"] {cursor: pointer;}:disabled {cursor: default;}img, svg, video, canvas, audio, iframe, embed, object {display: block;vertical-align: middle;}img, video {max-width: 100%;height: auto;}[hidden]:where(:not([hidden="until-found"])) {display: none;}.container {width: 100%;}@media (min-width: 640px) {.container {max-width: 640px;}}@media (min-width: 768px) {.container {max-width: 768px;}}@media (min-width: 1024px) {.container {max-width: 1024px;}}@media (min-width: 1280px) {.container {max-width: 1280px;}}@media (min-width: 1536px) {.container {max-width: 1536px;}}.sr-only {position: absolute;width: 1px;height: 1px;padding: 0;margin: -1px;overflow: hidden;clip: rect(0, 0, 0, 0);white-space: nowrap;border-width: 0;}.not-sr-only {position: static;width: auto;height: auto;padding: 0;margin: 0;overflow: visible;clip: auto;white-space: normal;}.pointer-events-none {pointer-events: none;}.pointer-events-auto {pointer-events: auto;}.\!visible {visibility: visible !important;}.visible {visibility: visible;}.invisible {visibility: hidden;}.collapse {visibility: collapse;}.static {position: static;}.fixed {position: fixed;}.absolute {position: absolute;}.relative {position: relative;}.sticky {position: sticky;}.-inset-1 {inset: -0.25rem;}.end-1 {inset-inline-end: 0.25rem;}.bottom-0 {bottom: 0px;}.isolate {isolation: isolate;}.isolation-auto {isolation: auto;}.float-start {float: inline-start;}.float-end {float: inline-end;}.float-right {float: right;}.float-left {float: left;}.float-none {float: none;}.clear-start {clear: inline-start;}.clear-end {clear: inline-end;}.clear-left {clear: left;}.clear-right {clear: right;}.clear-both {clear: both;}.clear-none {clear: none;}.mt-10 {margin-top: 2.5rem;}.mt-2 {margin-top: 0.5rem;}.mb-5 {margin-bottom: 1.25rem;}.mt-8 {margin-top: 2rem;}.mt-5 {margin-top: 1.25rem;}.mt-20 {margin-top: 5rem;}.mt-52 {margin-top: 13rem;}.box-border {box-sizing: border-box;}.box-content {box-sizing: content-box;}.line-clamp-none {overflow: visible;display: block;-webkit-box-orient: horizontal;-webkit-line-clamp: none;}.block {display: block;}.inline-block {display: inline-block;}.inline {display: inline;}.flex {display: flex;}.inline-flex {display: inline-flex;}.table {display: table;}.inline-table {display: inline-table;}.table-caption {display: table-caption;}.table-cell {display: table-cell;}.table-column {display: table-column;}.table-column-group {display: table-column-group;}.table-footer-group {display: table-footer-group;}.table-header-group {display: table-header-group;}.table-row-group {display: table-row-group;}.table-row {display: table-row;}.flow-root {display: flow-root;}.grid {display: grid;}.inline-grid {display: inline-grid;}.contents {display: contents;}.list-item {display: list-item;}.\!hidden {display: none !important;}.hidden {display: none;}.aspect-square {aspect-ratio: 1 / 1;}.h-full {height: 100%;}.h-screen {height: 100vh;}.w-\[this-is\\\\\] {width: this-is\\;}.w-\[this-is\] {width: this-is;}.w-\[weird-and-invalid\] {width: weird-and-invalid;}.w-full {width: 100%;}.flex-shrink {flex-shrink: 1;}.shrink {flex-shrink: 1;}.flex-grow {flex-grow: 1;}.grow {flex-grow: 1;}.table-auto {table-layout: auto;}.table-fixed {table-layout: fixed;}.caption-top {caption-side: top;}.caption-bottom {caption-side: bottom;}.border-collapse {border-collapse: collapse;}.border-separate {border-collapse: separate;}.-rotate-90 {--tw-rotate: -90deg;transform: translate(var(--tw-translate-x), var(--tw-translate-y)) rotate(var(--tw-rotate)) skewX(var(--tw-skew-x)) skewY(var(--tw-skew-y)) scaleX(var(--tw-scale-x)) scaleY(var(--tw-scale-y));}.\!transform {transform: translate(var(--tw-translate-x), var(--tw-translate-y)) rotate(var(--tw-rotate)) skewX(var(--tw-skew-x)) skewY(var(--tw-skew-y)) scaleX(var(--tw-scale-x)) scaleY(var(--tw-scale-y)) !important;}.transform {transform: translate(var(--tw-translate-x), var(--tw-translate-y)) rotate(var(--tw-rotate)) skewX(var(--tw-skew-x)) skewY(var(--tw-skew-y)) scaleX(var(--tw-scale-x)) scaleY(var(--tw-scale-y));}.transform-cpu {transform: translate(var(--tw-translate-x), var(--tw-translate-y)) rotate(var(--tw-rotate)) skewX(var(--tw-skew-x)) skewY(var(--tw-skew-y)) scaleX(var(--tw-scale-x)) scaleY(var(--tw-scale-y));}.transform-gpu {transform: translate3d(var(--tw-translate-x), var(--tw-translate-y), 0) rotate(var(--tw-rotate)) skewX(var(--tw-skew-x)) skewY(var(--tw-skew-y)) scaleX(var(--tw-scale-x)) scaleY(var(--tw-scale-y));}.transform-none {transform: none;}.touch-auto {touch-action: auto;}.touch-none {touch-action: none;}.touch-pan-x {--tw-pan-x: pan-x;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pan-left {--tw-pan-x: pan-left;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pan-right {--tw-pan-x: pan-right;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pan-y {--tw-pan-y: pan-y;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pan-up {--tw-pan-y: pan-up;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pan-down {--tw-pan-y: pan-down;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-pinch-zoom {--tw-pinch-zoom: pinch-zoom;touch-action: var(--tw-pan-x) var(--tw-pan-y) var(--tw-pinch-zoom);}.touch-manipulation {touch-action: manipulation;}.select-none {-webkit-user-select: none;-moz-user-select: none;user-select: none;}.select-text {-webkit-user-select: text;-moz-user-select: text;user-select: text;}.select-all {-webkit-user-select: all;-moz-user-select: all;user-select: all;}.select-auto {-webkit-user-select: auto;-moz-user-select: auto;user-select: auto;}.resize-none {resize: none;}.resize-y {resize: vertical;}.resize-x {resize: horizontal;}.resize {resize: both;}.snap-none {scroll-snap-type: none;}.snap-x {scroll-snap-type: x var(--tw-scroll-snap-strictness);}.snap-y {scroll-snap-type: y var(--tw-scroll-snap-strictness);}.snap-both {scroll-snap-type: both var(--tw-scroll-snap-strictness);}.snap-mandatory {--tw-scroll-snap-strictness: mandatory;}.snap-proximity {--tw-scroll-snap-strictness: proximity;}.snap-start {scroll-snap-align: start;}.snap-end {scroll-snap-align: end;}.snap-center {scroll-snap-align: center;}.snap-align-none {scroll-snap-align: none;}.snap-normal {scroll-snap-stop: normal;}.snap-always {scroll-snap-stop: always;}.list-inside {list-style-position: inside;}.list-outside {list-style-position: outside;}.appearance-none {-webkit-appearance: none;-moz-appearance: none;appearance: none;}.appearance-auto {-webkit-appearance: auto;-moz-appearance: auto;appearance: auto;}.break-before-auto {-moz-column-break-before: auto;break-before: auto;}.break-before-avoid {-moz-column-break-before: avoid;break-before: avoid;}.break-before-all {-moz-column-break-before: all;break-before: all;}.break-before-avoid-page {-moz-column-break-before: avoid;break-before: avoid-page;}.break-before-page {-moz-column-break-before: page;break-before: page;}.break-before-left {-moz-column-break-before: left;break-before: left;}.break-before-right {-moz-column-break-before: right;break-before: right;}.break-before-column {-moz-column-break-before: column;break-before: column;}.break-inside-auto {-moz-column-break-inside: auto;break-inside: auto;}.break-inside-avoid {-moz-column-break-inside: avoid;break-inside: avoid;}.break-inside-avoid-page {break-inside: avoid-page;}.break-inside-avoid-column {-moz-column-break-inside: avoid;break-inside: avoid-column;}.break-after-auto {-moz-column-break-after: auto;break-after: auto;}.break-after-avoid {-moz-column-break-after: avoid;break-after: avoid;}.break-after-all {-moz-column-break-after: all;break-after: all;}.break-after-avoid-page {-moz-column-break-after: avoid;break-after: avoid-page;}.break-after-page {-moz-column-break-after: page;break-after: page;}.break-after-left {-moz-column-break-after: left;break-after: left;}.break-after-right {-moz-column-break-after: right;break-after: right;}.break-after-column {-moz-column-break-after: column;break-after: column;}.grid-flow-row {grid-auto-flow: row;}.grid-flow-col {grid-auto-flow: column;}.grid-flow-dense {grid-auto-flow: dense;}.grid-flow-row-dense {grid-auto-flow: row dense;}.grid-flow-col-dense {grid-auto-flow: column dense;}.flex-row {flex-direction: row;}.flex-row-reverse {flex-direction: row-reverse;}.flex-col {flex-direction: column;}.flex-col-reverse {flex-direction: column-reverse;}.flex-wrap {flex-wrap: wrap;}.flex-wrap-reverse {flex-wrap: wrap-reverse;}.flex-nowrap {flex-wrap: nowrap;}.place-content-center {place-content: center;}.place-content-start {place-content: start;}.place-content-end {place-content: end;}.place-content-between {place-content: space-between;}.place-content-around {place-content: space-around;}.place-content-evenly {place-content: space-evenly;}.place-content-baseline {place-content: baseline;}.place-content-stretch {place-content: stretch;}.place-items-start {place-items: start;}.place-items-end {place-items: end;}.place-items-center {place-items: center;}.place-items-baseline {place-items: baseline;}.place-items-stretch {place-items: stretch;}.content-normal {align-content: normal;}.content-center {align-content: center;}.content-start {align-content: flex-start;}.content-end {align-content: flex-end;}.content-between {align-content: space-between;}.content-around {align-content: space-around;}.content-evenly {align-content: space-evenly;}.content-baseline {align-content: baseline;}.content-stretch {align-content: stretch;}.items-start {align-items: flex-start;}.items-end {align-items: flex-end;}.items-center {align-items: center;}.items-baseline {align-items: baseline;}.items-stretch {align-items: stretch;}.justify-normal {justify-content: normal;}.justify-start {justify-content: flex-start;}.justify-end {justify-content: flex-end;}.justify-center {justify-content: center;}.justify-between {justify-content: space-between;}.justify-around {justify-content: space-around;}.justify-evenly {justify-content: space-evenly;}.justify-stretch {justify-content: stretch;}.justify-items-start {justify-items: start;}.justify-items-end {justify-items: end;}.justify-items-center {justify-items: center;}.justify-items-stretch {justify-items: stretch;}.space-x-2 > :not([hidden]) ~ :not([hidden]) {--tw-space-x-reverse: 0;margin-right: calc(0.5rem * var(--tw-space-x-reverse));margin-left: calc(0.5rem * calc(1 - var(--tw-space-x-reverse)));}.space-y-reverse > :not([hidden]) ~ :not([hidden]) {--tw-space-y-reverse: 1;}.space-x-reverse > :not([hidden]) ~ :not([hidden]) {--tw-space-x-reverse: 1;}.divide-x > :not([hidden]) ~ :not([hidden]) {--tw-divide-x-reverse: 0;border-right-width: calc(1px * var(--tw-divide-x-reverse));border-left-width: calc(1px * calc(1 - var(--tw-divide-x-reverse)));}.divide-y > :not([hidden]) ~ :not([hidden]) {--tw-divide-y-reverse: 0;border-top-width: calc(1px * calc(1 - var(--tw-divide-y-reverse)));border-bottom-width: calc(1px * var(--tw-divide-y-reverse));}.divide-y-reverse > :not([hidden]) ~ :not([hidden]) {--tw-divide-y-reverse: 1;}.divide-x-reverse > :not([hidden]) ~ :not([hidden]) {--tw-divide-x-reverse: 1;}.divide-solid > :not([hidden]) ~ :not([hidden]) {border-style: solid;}.divide-dashed > :not([hidden]) ~ :not([hidden]) {border-style: dashed;}.divide-dotted > :not([hidden]) ~ :not([hidden]) {border-style: dotted;}.divide-double > :not([hidden]) ~ :not([hidden]) {border-style: double;}.divide-none > :not([hidden]) ~ :not([hidden]) {border-style: none;}.place-self-auto {place-self: auto;}.place-self-start {place-self: start;}.place-self-end {place-self: end;}.place-self-center {place-self: center;}.place-self-stretch {place-self: stretch;}.self-auto {align-self: auto;}.self-start {align-self: flex-start;}.self-end {align-self: flex-end;}.self-center {align-self: center;}.self-stretch {align-self: stretch;}.self-baseline {align-self: baseline;}.justify-self-auto {justify-self: auto;}.justify-self-start {justify-self: start;}.justify-self-end {justify-self: end;}.justify-self-center {justify-self: center;}.justify-self-stretch {justify-self: stretch;}.overflow-auto {overflow: auto;}.overflow-hidden {overflow: hidden;}.overflow-clip {overflow: clip;}.overflow-visible {overflow: visible;}.overflow-scroll {overflow: scroll;}.overflow-x-auto {overflow-x: auto;}.overflow-y-auto {overflow-y: auto;}.overflow-x-hidden {overflow-x: hidden;}.overflow-y-hidden {overflow-y: hidden;}.overflow-x-clip {overflow-x: clip;}.overflow-y-clip {overflow-y: clip;}.overflow-x-visible {overflow-x: visible;}.overflow-y-visible {overflow-y: visible;}.overflow-x-scroll {overflow-x: scroll;}.overflow-y-scroll {overflow-y: scroll;}.overscroll-auto {overscroll-behavior: auto;}.overscroll-contain {overscroll-behavior: contain;}.overscroll-none {overscroll-behavior: none;}.overscroll-y-auto {overscroll-behavior-y: auto;}.overscroll-y-contain {overscroll-behavior-y: contain;}.overscroll-y-none {overscroll-behavior-y: none;}.overscroll-x-auto {overscroll-behavior-x: auto;}.overscroll-x-contain {overscroll-behavior-x: contain;}.overscroll-x-none {overscroll-behavior-x: none;}.scroll-auto {scroll-behavior: auto;}.scroll-smooth {scroll-behavior: smooth;}.truncate {overflow: hidden;text-overflow: ellipsis;white-space: nowrap;}.overflow-ellipsis {text-overflow: ellipsis;}.text-ellipsis {text-overflow: ellipsis;}.text-clip {text-overflow: clip;}.hyphens-none {-webkit-hyphens: none;hyphens: none;}.hyphens-manual {-webkit-hyphens: manual;hyphens: manual;}.hyphens-auto {-webkit-hyphens: auto;hyphens: auto;}.whitespace-normal {white-space: normal;}.whitespace-nowrap {white-space: nowrap;}.whitespace-pre {white-space: pre;}.whitespace-pre-line {white-space: pre-line;}.whitespace-pre-wrap {white-space: pre-wrap;}.whitespace-break-spaces {white-space: break-spaces;}.text-wrap {text-wrap: wrap;}.text-nowrap {text-wrap: nowrap;}.text-balance {text-wrap: balance;}.text-pretty {text-wrap: pretty;}.break-normal {overflow-wrap: normal;word-break: normal;}.break-words {overflow-wrap: break-word;}.break-all {word-break: break-all;}.break-keep {word-break: keep-all;}.rounded {border-radius: 0.25rem;}.rounded-full {border-radius: 9999px;}.rounded-b {border-bottom-right-radius: 0.25rem;border-bottom-left-radius: 0.25rem;}.rounded-e {border-start-end-radius: 0.25rem;border-end-end-radius: 0.25rem;}.rounded-l {border-top-left-radius: 0.25rem;border-bottom-left-radius: 0.25rem;}.rounded-r {border-top-right-radius: 0.25rem;border-bottom-right-radius: 0.25rem;}.rounded-s {border-start-start-radius: 0.25rem;border-end-start-radius: 0.25rem;}.rounded-t {border-top-left-radius: 0.25rem;border-top-right-radius: 0.25rem;}.rounded-bl {border-bottom-left-radius: 0.25rem;}.rounded-br {border-bottom-right-radius: 0.25rem;}.rounded-ee {border-end-end-radius: 0.25rem;}.rounded-es {border-end-start-radius: 0.25rem;}.rounded-se {border-start-end-radius: 0.25rem;}.rounded-ss {border-start-start-radius: 0.25rem;}.rounded-tl {border-top-left-radius: 0.25rem;}.rounded-tr {border-top-right-radius: 0.25rem;}.border {border-width: 1px;}.border-x {border-left-width: 1px;border-right-width: 1px;}.border-y {border-top-width: 1px;border-bottom-width: 1px;}.border-b {border-bottom-width: 1px;}.border-e {border-inline-end-width: 1px;}.border-l {border-left-width: 1px;}.border-r {border-right-width: 1px;}.border-s {border-inline-start-width: 1px;}.border-t {border-top-width: 1px;}.border-solid {border-style: solid;}.border-dashed {border-style: dashed;}.border-dotted {border-style: dotted;}.border-double {border-style: double;}.border-hidden {border-style: hidden;}.border-none {border-style: none;}.bg-\[rgb\(255\2c 0\2c 0\)\] {--tw-bg-opacity: 1;background-color: rgb(255 0 0 / var(--tw-bg-opacity));}.bg-gray-100 {--tw-bg-opacity: 1;background-color: rgb(243 244 246 / var(--tw-bg-opacity));}.bg-gray-200 {--tw-bg-opacity: 1;background-color: rgb(229 231 235 / var(--tw-bg-opacity));}.bg-zinc-800 {--tw-bg-opacity: 1;background-color: rgb(39 39 42 / var(--tw-bg-opacity));}.decoration-slice {-webkit-box-decoration-break: slice;box-decoration-break: slice;}.decoration-clone {-webkit-box-decoration-break: clone;box-decoration-break: clone;}.box-decoration-slice {-webkit-box-decoration-break: slice;box-decoration-break: slice;}.box-decoration-clone {-webkit-box-decoration-break: clone;box-decoration-break: clone;}.bg-fixed {background-attachment: fixed;}.bg-local {background-attachment: local;}.bg-scroll {background-attachment: scroll;}.bg-clip-border {background-clip: border-box;}.bg-clip-padding {background-clip: padding-box;}.bg-clip-content {background-clip: content-box;}.bg-clip-text {-webkit-background-clip: text;background-clip: text;}.bg-repeat {background-repeat: repeat;}.bg-no-repeat {background-repeat: no-repeat;}.bg-repeat-x {background-repeat: repeat-x;}.bg-repeat-y {background-repeat: repeat-y;}.bg-repeat-round {background-repeat: round;}.bg-repeat-space {background-repeat: space;}.bg-origin-border {background-origin: border-box;}.bg-origin-padding {background-origin: padding-box;}.bg-origin-content {background-origin: content-box;}.stroke-blue-500 {stroke: #3b82f6;}.stroke-green-500 {stroke: #22c55e;}.stroke-orange-500 {stroke: #f97316;}.stroke-red-500 {stroke: #ef4444;}.stroke-yellow-500 {stroke: #eab308;}.object-contain {-o-object-fit: contain;object-fit: contain;}.object-cover {-o-object-fit: cover;object-fit: cover;}.object-fill {-o-object-fit: fill;object-fit: fill;}.object-none {-o-object-fit: none;object-fit: none;}.object-scale-down {-o-object-fit: scale-down;object-fit: scale-down;}.p-1 {padding: 0.25rem;}.p-2 {padding: 0.5rem;}.px-1 {padding-left: 0.25rem;padding-right: 0.25rem;}.px-1\.5 {padding-left: 0.375rem;padding-right: 0.375rem;}.px-5 {padding-left: 1.25rem;padding-right: 1.25rem;}.text-left {text-align: left;}.text-center {text-align: center;}.text-right {text-align: right;}.text-justify {text-align: justify;}.text-start {text-align: start;}.text-end {text-align: end;}.align-baseline {vertical-align: baseline;}.align-top {vertical-align: top;}.align-middle {vertical-align: middle;}.align-bottom {vertical-align: bottom;}.align-text-top {vertical-align: text-top;}.align-text-bottom {vertical-align: text-bottom;}.align-sub {vertical-align: sub;}.align-super {vertical-align: super;}.text-2xl {font-size: 1.5rem;line-height: 2rem;}.font-bold {font-weight: 700;}.font-semibold {font-weight: 600;}.uppercase {text-transform: uppercase;}.lowercase {text-transform: lowercase;}.capitalize {text-transform: capitalize;}.normal-case {text-transform: none;}.italic {font-style: italic;}.not-italic {font-style: normal;}.normal-nums {font-variant-numeric: normal;}.ordinal {--tw-ordinal: ordinal;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.slashed-zero {--tw-slashed-zero: slashed-zero;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.lining-nums {--tw-numeric-figure: lining-nums;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.oldstyle-nums {--tw-numeric-figure: oldstyle-nums;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.proportional-nums {--tw-numeric-spacing: proportional-nums;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.tabular-nums {--tw-numeric-spacing: tabular-nums;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.diagonal-fractions {--tw-numeric-fraction: diagonal-fractions;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.stacked-fractions {--tw-numeric-fraction: stacked-fractions;font-variant-numeric: var(--tw-ordinal) var(--tw-slashed-zero) var(--tw-numeric-figure) var(--tw-numeric-spacing) var(--tw-numeric-fraction);}.text-\[\#336699\]\/\[\.35\] {color: rgb(51 102 153 / .35);}.text-blue-500 {--tw-text-opacity: 1;color: rgb(59 130 246 / var(--tw-text-opacity));}.text-gray-100 {--tw-text-opacity: 1;color: rgb(243 244 246 / var(--tw-text-opacity));}.text-gray-700 {--tw-text-opacity: 1;color: rgb(55 65 81 / var(--tw-text-opacity));}.text-red-500 {--tw-text-opacity: 1;color: rgb(239 68 68 / var(--tw-text-opacity));}.text-white {--tw-text-opacity: 1;color: rgb(255 255 255 / var(--tw-text-opacity));}.text-gray-500 {--tw-text-opacity: 1;color: rgb(107 114 128 / var(--tw-text-opacity));}.text-gray-400 {--tw-text-opacity: 1;color: rgb(156 163 175 / var(--tw-text-opacity));}.underline {text-decoration-line: underline;}.overline {text-decoration-line: overline;}.line-through {text-decoration-line: line-through;}.no-underline {text-decoration-line: none;}.decoration-solid {text-decoration-style: solid;}.decoration-double {text-decoration-style: double;}.decoration-dotted {text-decoration-style: dotted;}.decoration-dashed {text-decoration-style: dashed;}.decoration-wavy {text-decoration-style: wavy;}.antialiased {-webkit-font-smoothing: antialiased;-moz-osx-font-smoothing: grayscale;}.subpixel-antialiased {-webkit-font-smoothing: auto;-moz-osx-font-smoothing: auto;}.bg-blend-normal {background-blend-mode: normal;}.bg-blend-multiply {background-blend-mode: multiply;}.bg-blend-screen {background-blend-mode: screen;}.bg-blend-overlay {background-blend-mode: overlay;}.bg-blend-darken {background-blend-mode: darken;}.bg-blend-lighten {background-blend-mode: lighten;}.bg-blend-color-dodge {background-blend-mode: color-dodge;}.bg-blend-color-burn {background-blend-mode: color-burn;}.bg-blend-hard-light {background-blend-mode: hard-light;}.bg-blend-soft-light {background-blend-mode: soft-light;}.bg-blend-difference {background-blend-mode: difference;}.bg-blend-exclusion {background-blend-mode: exclusion;}.bg-blend-hue {background-blend-mode: hue;}.bg-blend-saturation {background-blend-mode: saturation;}.bg-blend-color {background-blend-mode: color;}.bg-blend-luminosity {background-blend-mode: luminosity;}.mix-blend-normal {mix-blend-mode: normal;}.mix-blend-multiply {mix-blend-mode: multiply;}.mix-blend-screen {mix-blend-mode: screen;}.mix-blend-overlay {mix-blend-mode: overlay;}.mix-blend-darken {mix-blend-mode: darken;}.mix-blend-lighten {mix-blend-mode: lighten;}.mix-blend-color-dodge {mix-blend-mode: color-dodge;}.mix-blend-color-burn {mix-blend-mode: color-burn;}.mix-blend-hard-light {mix-blend-mode: hard-light;}.mix-blend-soft-light {mix-blend-mode: soft-light;}.mix-blend-difference {mix-blend-mode: difference;}.mix-blend-exclusion {mix-blend-mode: exclusion;}.mix-blend-hue {mix-blend-mode: hue;}.mix-blend-saturation {mix-blend-mode: saturation;}.mix-blend-color {mix-blend-mode: color;}.mix-blend-luminosity {mix-blend-mode: luminosity;}.mix-blend-plus-darker {mix-blend-mode: plus-darker;}.mix-blend-plus-lighter {mix-blend-mode: plus-lighter;}.\!shadow {--tw-shadow: 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1) !important;--tw-shadow-colored: 0 1px 3px 0 var(--tw-shadow-color), 0 1px 2px -1px var(--tw-shadow-color) !important;box-shadow: var(--tw-ring-offset-shadow, 0 0 #0000), var(--tw-ring-shadow, 0 0 #0000), var(--tw-shadow) !important;}.shadow {--tw-shadow: 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1);--tw-shadow-colored: 0 1px 3px 0 var(--tw-shadow-color), 0 1px 2px -1px var(--tw-shadow-color);box-shadow: var(--tw-ring-offset-shadow, 0 0 #0000), var(--tw-ring-shadow, 0 0 #0000), var(--tw-shadow);}.shadow-md {--tw-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1);--tw-shadow-colored: 0 4px 6px -1px var(--tw-shadow-color), 0 2px 4px -2px var(--tw-shadow-color);box-shadow: var(--tw-ring-offset-shadow, 0 0 #0000), var(--tw-ring-shadow, 0 0 #0000), var(--tw-shadow);}.outline-none {outline: 2px solid transparent;outline-offset: 2px;}.outline {outline-style: solid;}.outline-dashed {outline-style: dashed;}.outline-dotted {outline-style: dotted;}.outline-double {outline-style: double;}.ring {--tw-ring-offset-shadow: var(--tw-ring-inset) 0 0 0 var(--tw-ring-offset-width) var(--tw-ring-offset-color);--tw-ring-shadow: var(--tw-ring-inset) 0 0 0 calc(3px + var(--tw-ring-offset-width)) var(--tw-ring-color);box-shadow: var(--tw-ring-offset-shadow), var(--tw-ring-shadow), var(--tw-shadow, 0 0 #0000);}.ring-inset {--tw-ring-inset: inset;}.blur {--tw-blur: blur(8px);filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.drop-shadow {--tw-drop-shadow: drop-shadow(0 1px 2px rgb(0 0 0 / 0.1)) drop-shadow(0 1px 1px rgb(0 0 0 / 0.06));filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.grayscale {--tw-grayscale: grayscale(100%);filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.invert {--tw-invert: invert(100%);filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.sepia {--tw-sepia: sepia(100%);filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.\!filter {filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow) !important;}.filter {filter: var(--tw-blur) var(--tw-brightness) var(--tw-contrast) var(--tw-grayscale) var(--tw-hue-rotate) var(--tw-invert) var(--tw-saturate) var(--tw-sepia) var(--tw-drop-shadow);}.filter-none {filter: none;}.backdrop-blur {--tw-backdrop-blur: blur(8px);-webkit-backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);}.backdrop-grayscale {--tw-backdrop-grayscale: grayscale(100%);-webkit-backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);}.backdrop-invert {--tw-backdrop-invert: invert(100%);-webkit-backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);}.backdrop-sepia {--tw-backdrop-sepia: sepia(100%);-webkit-backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);}.backdrop-filter {-webkit-backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);backdrop-filter: var(--tw-backdrop-blur) var(--tw-backdrop-brightness) var(--tw-backdrop-contrast) var(--tw-backdrop-grayscale) var(--tw-backdrop-hue-rotate) var(--tw-backdrop-invert) var(--tw-backdrop-opacity) var(--tw-backdrop-saturate) var(--tw-backdrop-sepia);}.backdrop-filter-none {-webkit-backdrop-filter: none;backdrop-filter: none;}.transition {transition-property: color, background-color, border-color, text-decoration-color, fill, stroke, opacity, box-shadow, transform, filter, -webkit-backdrop-filter;transition-property: color, background-color, border-color, text-decoration-color, fill, stroke, opacity, box-shadow, transform, filter, backdrop-filter;transition-property: color, background-color, border-color, text-decoration-color, fill, stroke, opacity, box-shadow, transform, filter, backdrop-filter, -webkit-backdrop-filter;transition-timing-function: cubic-bezier(0.4, 0, 0.2, 1);transition-duration: 150ms;}.ease-in {transition-timing-function: cubic-bezier(0.4, 0, 1, 1);}.ease-in-out {transition-timing-function: cubic-bezier(0.4, 0, 0.2, 1);}.ease-out {transition-timing-function: cubic-bezier(0, 0, 0.2, 1);}.contain-none {contain: none;}.contain-content {contain: content;}.contain-strict {contain: strict;}.contain-size {--tw-contain-size: size;contain: var(--tw-contain-size) var(--tw-contain-layout) var(--tw-contain-paint) var(--tw-contain-style);}.contain-inline-size {--tw-contain-size: inline-size;contain: var(--tw-contain-size) var(--tw-contain-layout) var(--tw-contain-paint) var(--tw-contain-style);}.contain-layout {--tw-contain-layout: layout;contain: var(--tw-contain-size) var(--tw-contain-layout) var(--tw-contain-paint) var(--tw-contain-style);}.contain-paint {--tw-contain-paint: paint;contain: var(--tw-contain-size) var(--tw-contain-layout) var(--tw-contain-paint) var(--tw-contain-style);}.contain-style {--tw-contain-style: style;contain: var(--tw-contain-size) var(--tw-contain-layout) var(--tw-contain-paint) var(--tw-contain-style);}.content-\[\'this-is-also-valid\]-weirdly-enough\'\] {--tw-content: 'this-is-also-valid]-weirdly-enough';content: var(--tw-content);}.forced-color-adjust-auto {forced-color-adjust: auto;}.forced-color-adjust-none {forced-color-adjust: none;}@media (min-width: 640px) {.sm\:container {width: 100%;}@media (min-width: 640px) {.sm\:container {max-width: 640px;}}@media (min-width: 768px) {.sm\:container {max-width: 768px;}}@media (min-width: 1024px) {.sm\:container {max-width: 1024px;}}@media (min-width: 1280px) {.sm\:container {max-width: 1280px;}}@media (min-width: 1536px) {.sm\:container {max-width: 1536px;}}}.hover\:bg-gray-300:hover {--tw-bg-opacity: 1;background-color: rgb(209 213 219 / var(--tw-bg-opacity));}.hover\:font-bold:hover {font-weight: 700;}.before\:hover\:text-center:hover::before {content: var(--tw-content);text-align: center;}.hover\:before\:text-center:hover::before {content: var(--tw-content);text-align: center;}.focus\:outline-none:focus {outline: 2px solid transparent;outline-offset: 2px;}.focus\:hover\:text-center:hover:focus {text-align: center;}.hover\:focus\:text-center:focus:hover {text-align: center;}.dark\:block:is(.dark *) {display: block;}.dark\:hidden:is(.dark *) {display: none;}.dark\:text-black:is(.dark *) {--tw-text-opacity: 1;color: rgb(0 0 0 / var(--tw-text-opacity));}.dark\:text-gray-100:is(.dark *) {--tw-text-opacity: 1;color: rgb(243 244 246 / var(--tw-text-opacity));}@media (min-width: 640px) {.sm\:underline {text-decoration-line: underline;}}@media (min-width: 768px) {.md\:mt-5 {margin-top: 1.25rem;}}@media (min-width: 1024px) {.lg\:mt-10 {margin-top: 2.5rem;}.dark\:lg\:hover\:\[paint-order\:markers\]:hover:is(.dark *) {paint-order: markers;}})";

// Serve the output.css
void htmlCSS() {
  server.send(200, "text/css", css);
}

// Serve the script.js
void htmlScript() {
  String script;
  script += "function updateProgress(value, id) {";
  // Get the circle and text elements by their IDs
  script += "const circle = document.getElementById(id);";
  script += "const text = document.getElementById(id + '-text');";
  // Calculate the circumference of the circle
  script += "const radius = circle.r.baseVal.value;";
  script += "const circumference = 2 * Math.PI * radius;";
  script += "let offset;";
  // Determine the offset based on the value and id
  script += "if (value < 0) {";
  script += "offset = circumference;";
  script += "} else if ((id === 't' && value > 40) || (id === 'h' && value > 100)) {";
  script += "offset = 0;";
  script += "} else { offset = circumference - (value / (id === 't' ? 40 : 100)) * circumference; }";
  // Set the stroke dash offset to create the progress effect
  script += "circle.style.strokeDashoffset = offset;";
  // Define the conditions for temperature and humidity
  script += "const conditions = {";
  script += "t: [";
  script += "{ max: 18, color: 'stroke-blue-500', label: '(Cold)' },";
  script += "{ max: 24, color: 'stroke-green-500', label: '(Comfortable)' },";
  script += "{ max: 28, color: 'stroke-yellow-500', label: '(Warm)' },";
  script += "{ max: 32, color: 'stroke-orange-500', label: '(Hot)' },";
  script += "{ max: Infinity, color: 'stroke-red-500', label: '(Very Hot)' }";
  script += "],";
  script += "h: [";
  script += "{ max: 30, color: 'stroke-blue-500', label: '(Dry)' },";
  script += "{ max: 60, color: 'stroke-green-500', label: '(Comfortable)' },";
  script += "{ max: 70, color: 'stroke-yellow-500', label: '(Humid)' },";
  script += "{ max: 80, color: 'stroke-orange-500', label: '(Very Humid)' },";
  script += "{ max: Infinity, color: 'stroke-red-500', label: '(Excessive)' }";
  script += "] };";
  // Find the appropriate condition based on the value
  script += "const condition = conditions[id].find(cond => (value <= cond.max));";
  // Remove previous color classes that start with 'stroke-'
  script += "circle.classList.forEach(cls => {";
  script += "if (cls.startsWith('stroke-')) {";
  script += "circle.classList.remove(cls);";
  script += "} });";
  // Add the new color class
  script += "circle.classList.add(condition.color);";
  // Update the text content with the value and condition label
  script += "text.innerHTML = value + (id === 't' ? '&#8451;' : '&#37') + '<br>' + condition.label; }";
  // Get the status text by their element ID
  script += "statusText = document.getElementById('status');";
  // Fetch data from the API, update the progress and change the status every 3 seconds
  script += "function getTempAndHumidity() {";
  script += "fetch('/api')";
  script += ".then(response => response.json())";
  script += ".then(data => {";
  script += "updateProgress(data.temperature, 't');";
  script += "updateProgress(data.humidity, 'h');";
  script += "statusText.innerHTML = 'Connected';";
  script += "statusText.style.color = 'green';";
  script += "})";
  script += ".catch(error => {";
  script += "console.error('Error fetching data:', error);";
  script += "statusText.innerHTML = 'Disconnected';";
  script += "statusText.style.color = 'red';";
  script += "});}";
  script += "setInterval(() => {";
  script += "  getTempAndHumidity();";
  script += "}, 3000);";
  script += "getTempAndHumidity();";
  // Check the local storage for theme preference on page load
  script += "if (localStorage.getItem('theme') === 'dark') {";
  script += "document.body.classList.add('dark', 'bg-zinc-800', 'text-gray-100');";
  script += "} else {";
  script += "document.body.classList.add('bg-gray-100');}";
  // Get the theme toggle button
  script += "const themeToggle = document.getElementById('theme-toggle');";
  // Toggle the theme when the button is clicked
  script += "themeToggle.addEventListener('click', () => {";
  script += "document.body.classList.toggle('dark');";
  script += "document.body.classList.toggle('bg-gray-100');";
  script += "document.body.classList.toggle('bg-zinc-800');";
  script += "document.body.classList.toggle('text-gray-100');";
  // Save the theme preference to local storage
  script += "if (document.body.classList.contains('dark')) {";
  script += "localStorage.setItem('theme', 'dark');";
  script += "} else {";
  script += "localStorage.setItem('theme', 'light');";
  script += "}});";
  server.send(200, "application/javascript", script);
}


// Serve data (temperature + humidity)
void api() {
  String data = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
  // Allow any origin to access the api
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", data);
}

// Ping a computer to check if it's online
void pingComputer() {
  String ip = server.arg("ip");
  bool isOnline = false;
  
  if (ip.length() > 0) {
    // Simple connection test instead of ping
    IPAddress target;
    if (target.fromString(ip)) {
      // Try to connect to common ports to check if the device is online
      WiFiClient client;
      client.setTimeout(500); // 500ms timeout
      
      // Try common ports: 80 (HTTP), 443 (HTTPS), 22 (SSH)
      int ports[] = {80, 443, 22};
      for (int i = 0; i < 3; i++) {
        if (client.connect(target, ports[i])) {
          client.stop();
          isOnline = true;
          break;
        }
        delay(100);
      }
    }
  }
  
  String response = "{\"online\":" + String(isOnline ? "true" : "false") + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

// Send Wake on LAN magic packet
void wakeOnLAN() {
  // Get request body
  String body = server.arg("plain");
  
  // Parse JSON
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }
  
  // Get MAC address from request
  String macStr = doc["mac"].as<String>();
  
  // Convert MAC string to bytes (format: XX-XX-XX-XX-XX-XX)
  byte mac[6];
  if (!macAddressToBytes(macStr, mac)) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid MAC address format\"}");
    return;
  }
  
  // Send Wake on LAN magic packet
  bool success = sendWOL(mac);
  
  String response = "{\"success\":" + String(success ? "true" : "false") + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

// Convert MAC address string to bytes
bool macAddressToBytes(String macStr, byte* mac) {
  // Replace "-" with ":"
  macStr.replace("-", ":");
  
  // Check if the MAC address is valid
  if (macStr.length() != 17) {
    return false;
  }
  
  // Convert MAC address string to bytes
  for (int i = 0; i < 6; i++) {
    int index = i * 3;
    String byteString = macStr.substring(index, index + 2);
    mac[i] = strtol(byteString.c_str(), NULL, 16);
  }
  
  return true;
}

// Send Wake on LAN magic packet
bool sendWOL(byte* mac) {
  // Create magic packet
  byte magicPacket[102];
  
  // First 6 bytes of 0xFF
  for (int i = 0; i < 6; i++) {
    magicPacket[i] = 0xFF;
  }
  
  // Repeat MAC address 16 times
  for (int i = 0; i < 16; i++) {
    int offset = i * 6 + 6;
    for (int j = 0; j < 6; j++) {
      magicPacket[offset + j] = mac[j];
    }
  }
  
  // Send packet to broadcast address
  udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
  udp.write(magicPacket, sizeof(magicPacket));
  return udp.endPacket() == 1;
}

// SSH to a computer and send shutdown command
void shutdownComputer() {
  // Get request body
  String body = server.arg("plain");
  
  // Parse JSON
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }
  
  // Get IP address, username and password from request
  String ip = doc["ip"].as<String>();
  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();
  
  // Validate parameters
  if (ip.length() == 0 || username.length() == 0 || password.length() == 0) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    return;
  }
  
  // Note: Implementing a proper SSH client on ESP32 requires additional libraries
  // This is a simplified implementation for demonstration purposes
  
  // For a real implementation, you would use something like:
  // 1. ESP32 SSH client library (if available)
  // 2. Set up a service on your network that can receive HTTP requests
  //    and execute SSH commands on your behalf
  
  // For now, we'll simulate success but log a message
  Serial.println("Shutdown request received for:");
  Serial.println("IP: " + ip);
  Serial.println("Username: " + username);
  
  // Return a success response (simulated)
  String response = "{\"success\":true,\"message\":\"Shutdown command sent (simulated)\"}";
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

// Serve 404 page
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  // List arguments
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up http://esp32.local
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  // Create routes
  server.on("/", htmlIndex);
  server.on("/favicon.svg", htmlFavicon);
  server.on("/output.css", htmlCSS);
  server.on("/script.js", htmlScript);
  server.on("/api", api);
  server.on("/ping", pingComputer);
  server.on("/wol", wakeOnLAN);
  server.on("/ssh_shutdown", shutdownComputer);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);
  // Read data from the module
  dht11.read(&temperature, &humidity, NULL);
}
