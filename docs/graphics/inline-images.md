---
layout: default
title: "Inline Images"
category: "GRAPHICS"
status: "implemented"
---

<h2 id="pic-command">The pic Command</h2>
<p>Meridian provides the built-in <code>pic</code> command for instant, hardware-rendered raster image viewing directly within the terminal canvas:</p>
<pre><code class="language-bash">pic image.png              # Display image at default canvas coordinates
pic screenshot.jpg --width 400  # Scale image to 400px width maintaining aspect ratio
pic --debug photo.webp     # Inspect image dimensions, channels, and texture format
pic --clear                # Clear all active raster images from canvas</code></pre>

<h2 id="zero-ascii-guarantee">Direct Raster Rendering (Zero ASCII)</h2>
<p>Unlike legacy terminal scripts that convert pictures into ASCII or colored block characters, Meridian's <code>pic</code> command decodes raw image bytes into 32-bit RGBA pixel buffers and renders them directly on the GPU canvas with zero downsampling or block approximations.</p>

<h2 id="dimensions-scaling">Aspect Fitting & Scaling</h2>
<p><code>graphics::ImageDecoder</code> automatically computes aspect ratios to prevent image distortion when scaling across varying terminal window dimensions.</p>

<h2 id="cli-flags">Command-Line Options & Flags</h2>
<table class="doc-table">
  <thead><tr><th>Flag</th><th>Description</th></tr></thead>
  <tbody>
    <tr><td><code>--width &lt;px&gt;</code></td><td>Set target image width in pixels.</td></tr>
    <tr><td><code>--height &lt;px&gt;</code></td><td>Set target image height in pixels.</td></tr>
    <tr><td><code>--debug</code></td><td>Print image format, dimensions, and GPU texture ID.</td></tr>
    <tr><td><code>--clear</code></td><td>Remove all rendered inline images.</td></tr>
  </tbody>
</table>