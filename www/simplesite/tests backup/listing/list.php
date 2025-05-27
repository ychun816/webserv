<?php
header('Content-Type: text/html; charset=utf-8');
$path = isset($_GET['path']) ? $_GET['path'] : '.';
$path = realpath($path);
if (!$path || !is_dir($path)) {
  $path = '.';
}
$files = scandir($path);
usort($files, function($a, $b) use ($path) {
  return (is_dir("$path/$b") <=> is_dir("$path/$a")) ?: strcasecmp($a, $b);
});
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <!-- Same head section with CSS as above -->
</head>
<body>
  <div class="container">
    <h1>📂 Directory Listing</h1>
    <ul>
      <?php if ($path !== realpath('.')): ?>
      <li><a href="?path=<?= urlencode(dirname($path)) ?>">../ (Parent Directory)</a></li>
      <?php endif; ?>
      <?php foreach ($files as $file):
        if ($file === '.' || $file === '..') continue;
        $fullPath = "$path/$file";
        $isDir = is_dir($fullPath);
        $displayName = $file . ($isDir ? '/' : '');
        $link = "?path=" . urlencode($fullPath);
      ?>
      <li><a href="<?= $link ?>"><?= htmlspecialchars($displayName) ?></a></li>
      <?php endforeach; ?>
    </ul>
    <div class="nav-links">
      <a href="/">🏠 Root</a>
      <a href="/upload/">📤 Upload</a>
      <a href="/upload/download.html">📥 Download</a>
      <a href="/delete.html">🗑️ Delete</a>
    </div>
  </div>
</body>
</html>
