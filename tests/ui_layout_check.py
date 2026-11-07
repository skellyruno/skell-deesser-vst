#!/usr/bin/env python3
"""JUCE-independent source/UI geometry smoke test; not a visual/host test."""
import re
from pathlib import Path
root = Path(__file__).resolve().parent.parent
editor = (root / 'Source/PluginEditor.cpp').read_text()
processor = (root / 'Source/PluginProcessor.cpp').read_text()
controls = (root / 'Source/ui/Controls.h').read_text()
look = (root / 'Source/ui/LookAndFeel.h').read_text()
assert 'constexpr int compactWidth=633, compactHeight=422' in editor
assert '"editorWidth",633' in processor and '"editorHeight",422' in processor
assert 'compactLayoutV7' in editor and 'compactLayoutV7' in processor
assert 'getHeight()*.53f' in controls
assert 'getHeight()*.14f' in controls
assert 'phraseButton.setToggleState(phraseOn' in editor
assert 'phraseAware' in processor and 'phraseSensitivity' in processor
assert 'title(g,"THRESHOLD"' in editor
assert 'g.drawFittedText("SC LEVEL"' in editor
assert 'if(!std::isfinite(v))return;' in editor
assert '"SAVE"' in look and 'const float side=juce::jmin(b.getWidth(),b.getHeight())*.60f' in look
# Source declares each component's base coordinates in resized().
placements = {}
for name, x,y,w,h in re.findall(r'place\((\w+),(\d+),(\d+),(\d+),(\d+)\)', editor):
    placements[name] = tuple(map(int,(x,y,w,h)))
for key in ('detectHeader','frequencyHeader','sideHeader','monitorHeader',
            'split','wide','phraseButton','audio','sidechain','frequency','frequencyValue',
            'threshold','thresholdValue','prev','preset','next','save','settings'):
    assert key in placements, f'missing component: {key}'
for name, (x,y,w,h) in placements.items():
    assert x>=0 and y>=0 and w>0 and h>0 and x+w<=1536 and y+h<=1024, name
for a,b in [('prev','preset'),('preset','next'),('next','save'),('save','settings')]:
    ax,ay,aw,ah=placements[a]; bx,by,bw,bh=placements[b]
    assert ax+aw<=bx, f'{a} overlaps {b}'
for a,b in [('detectHeader','split'),('split','wide'),('wide','phraseButton'),('phraseButton','frequencyHeader'),
            ('frequencyHeader','frequency'),('frequency','frequencyValue'),
            ('frequencyValue','sideHeader'),('sideHeader','hpf'),
            ('hpf','monitorHeader'),('monitorHeader','audio')]:
    # hpf sits alongside filter, and headings are stacked below in Y.
    ax,ay,aw,ah=placements[a]; bx,by,bw,bh=placements[b]
    assert ay+ah<=by, f'{a} overlaps {b}'
for width,height in [(540,360),(633,422),(806,537),(979,653),(1152,768),(1536,1024)]:
    assert abs(width/height-1.5)<.005
    for name,(x,y,w,h) in placements.items():
        scaled=(round(x*width/1536),round(y*height/1024),round(w*width/1536),round(h*height/1024))
        assert scaled[2]>0 and scaled[3]>0, name
print('UI geometry source checks passed (including compact 633x422 layout)')
