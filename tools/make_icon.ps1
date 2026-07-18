Add-Type -AssemblyName System.Drawing
$outDir = Join-Path $PSScriptRoot '..\res'
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory $outDir | Out-Null }
$outPath = Join-Path $outDir 'gangland.ico'
$sizes = @(16, 24, 32, 48, 256)
$pngs = @()
foreach ($s in $sizes) {
  $bmp = New-Object System.Drawing.Bitmap($s, $s)
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
  $g.Clear([System.Drawing.Color]::Transparent)
  $bg = [System.Drawing.Color]::FromArgb(255, 24, 23, 29)
  $gold = [System.Drawing.Color]::FromArgb(255, 205, 165, 66)
  $blood = [System.Drawing.Color]::FromArgb(255, 128, 34, 34)
  $bgBrush = New-Object System.Drawing.SolidBrush($bg)
  $goldBrush = New-Object System.Drawing.SolidBrush($gold)
  $bloodBrush = New-Object System.Drawing.SolidBrush($blood)
  $penW = [single][Math]::Max(1.0, $s / 20.0)
  $goldPen = New-Object System.Drawing.Pen($gold, $penW)
  $inset = [single]($penW / 2 + $s * 0.02)
  $g.FillEllipse($bgBrush, $inset, $inset, [single]($s - 2 * $inset), [single]($s - 2 * $inset))
  $g.DrawEllipse($goldPen, $inset, $inset, [single]($s - 2 * $inset), [single]($s - 2 * $inset))
  $g.FillEllipse($goldBrush, [single]($s * 0.33), [single]($s * 0.17), [single]($s * 0.34), [single]($s * 0.14))
  $g.FillRectangle($goldBrush, [single]($s * 0.33), [single]($s * 0.24), [single]($s * 0.34), [single]($s * 0.26))
  $g.FillRectangle($bloodBrush, [single]($s * 0.32), [single]($s * 0.42), [single]($s * 0.36), [single]($s * 0.10))
  $g.FillEllipse($goldBrush, [single]($s * 0.18), [single]($s * 0.50), [single]($s * 0.64), [single]($s * 0.14))
  $g.Dispose()
  $ms = New-Object System.IO.MemoryStream
  $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
  $pngs += , @($s, $ms.ToArray())
  $bmp.Dispose()
}
$msOut = New-Object System.IO.MemoryStream
$bw = New-Object System.IO.BinaryWriter($msOut)
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$pngs.Count)
$offset = 6 + 16 * $pngs.Count
foreach ($p in $pngs) {
  $s = $p[0]; $data = $p[1]
  $dim = if ($s -ge 256) { 0 } else { $s }
  $bw.Write([Byte]$dim); $bw.Write([Byte]$dim)
  $bw.Write([Byte]0); $bw.Write([Byte]0)
  $bw.Write([UInt16]1); $bw.Write([UInt16]32)
  $bw.Write([UInt32]$data.Length); $bw.Write([UInt32]$offset)
  $offset += $data.Length
}
foreach ($p in $pngs) { $bw.Write($p[1]) }
$bw.Flush()
[System.IO.File]::WriteAllBytes($outPath, $msOut.ToArray())
Write-Output "ICO written: $outPath ($($msOut.Length) bytes)"
