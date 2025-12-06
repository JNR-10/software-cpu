let trace = [];
let sourceMap = [];
let currentCycle = 0;
let isPlaying = false;
let playInterval = null;
let playSpeed = 1000; // milliseconds between cycles

// Initialize application
document.addEventListener('DOMContentLoaded', () => {
  loadTraceList();

  // Setup Back Button
  document.getElementById('back-btn').addEventListener('click', () => {
    stopPlayback();
    document.getElementById('viewer-container').classList.add('hidden');
    document.getElementById('trace-selection').classList.remove('hidden');
    window.history.pushState({}, document.title, window.location.pathname);
  });

  // Setup Slider
  document.getElementById('slider').addEventListener('input', function (e) {
    const cycle = parseInt(e.target.value, 10);
    currentCycle = cycle;
    render(cycle);
  });

  // Setup Playback Controls
  document.getElementById('first-btn').addEventListener('click', () => {
    stopPlayback();
    goToFirst();
  });

  document.getElementById('prev-btn').addEventListener('click', () => {
    stopPlayback();
    goToPrevious();
  });

  document.getElementById('play-pause-btn').addEventListener('click', togglePlayback);

  document.getElementById('next-btn').addEventListener('click', () => {
    stopPlayback();
    goToNext();
  });

  document.getElementById('last-btn').addEventListener('click', () => {
    stopPlayback();
    goToLast();
  });

  document.getElementById('reset-btn').addEventListener('click', () => {
    stopPlayback();
    goToFirst();
  });

  // Setup Speed Control
  document.getElementById('speed-select').addEventListener('change', function (e) {
    playSpeed = parseInt(e.target.value, 10);
    if (isPlaying) {
      // Restart with new speed
      stopPlayback();
      startPlayback();
    }
  });

  // Keyboard shortcuts
  document.addEventListener('keydown', (e) => {
    if (document.getElementById('viewer-container').classList.contains('hidden')) return;

    switch (e.key) {
      case ' ':
        e.preventDefault();
        togglePlayback();
        break;
      case 'ArrowRight':
        e.preventDefault();
        stopPlayback();
        goToNext();
        break;
      case 'ArrowLeft':
        e.preventDefault();
        stopPlayback();
        goToPrevious();
        break;
      case 'Home':
        e.preventDefault();
        stopPlayback();
        goToFirst();
        break;
      case 'End':
        e.preventDefault();
        stopPlayback();
        goToLast();
        break;
    }
  });

  // Check URL params for direct link
  const params = new URLSearchParams(window.location.search);
  const traceFile = params.get('trace');
  if (traceFile) {
    loadTrace(traceFile);
  }
});

// Fetch and display list of available traces
function loadTraceList() {
  const listContainer = document.getElementById('trace-list');

  fetch('../build/traces/traces.json')
    .then(response => {
      if (!response.ok) throw new Error('Manifest not found');
      return response.json();
    })
    .then(files => {
      listContainer.innerHTML = '';
      if (files.length === 0) {
        listContainer.innerHTML = '<p>No traces found. Run demo.sh to generate them.</p>';
        return;
      }

      files.forEach(file => {
        const btn = document.createElement('button');
        btn.className = 'trace-btn';

        // Extract the base name without timestamp and .json extension
        const baseName = file.replace('.json', '').replace(/_\d{8}_\d{6}$/, '');

        // Format for display: replace underscores with spaces and capitalize
        const displayName = baseName.replace(/_/g, ' ')
          .split(' ')
          .map(word => word.charAt(0).toUpperCase() + word.slice(1))
          .join(' ');

        btn.innerHTML = `<div class="trace-icon">📊</div><div class="trace-name">${displayName}</div>`;
        btn.dataset.originalName = baseName; // Store original name
        btn.onclick = () => loadTrace(file, baseName);
        listContainer.appendChild(btn);
      });
    })
    .catch(err => {
      console.error('Error loading trace list:', err);
      listContainer.innerHTML = '<p>Error loading traces. Make sure you are running via demo.sh</p>';
    });
}

// Load a specific trace file
function loadTrace(filename, baseName) {
  const path = '../build/traces/' + filename;
  const mapPath = path.replace('.json', '.map.json');

  fetch(path)
    .then(response => {
      if (!response.ok) throw new Error('Failed to load trace file');
      return response.json();
    })
    .then(data => {
      trace = data;
      currentCycle = 0;

      // Update UI
      document.getElementById('trace-selection').classList.add('hidden');
      document.getElementById('viewer-container').classList.remove('hidden');

      // Use the base name for consistent display
      const displayName = baseName.replace(/_/g, ' ')
        .split(' ')
        .map(word => word.charAt(0).toUpperCase() + word.slice(1))
        .join(' ');
      document.getElementById('current-trace-name').textContent = displayName;

      // Setup Slider
      const maxCycle = Math.max(0, trace.length - 1);
      document.getElementById('slider').max = maxCycle;
      document.getElementById('slider').value = 0;

      // Update stats
      document.getElementById('total-cycles').textContent = `Total Cycles: ${trace.length}`;
      updateProgress();

      // Update URL without reloading
      const newUrl = new URL(window.location);
      newUrl.searchParams.set('trace', filename);
      window.history.pushState({}, '', newUrl);

      // Load Source Map
      fetch(mapPath)
        .then(r => {
          if (r.ok) return r.json();
          return [];
        })
        .then(map => {
          sourceMap = map;
          renderCodeView();
          render(0);
        })
        .catch(e => {
          console.log("No source map found");
          sourceMap = [];
          renderCodeView();
          render(0);
        });
    })
    .catch(err => {
      console.error('Error loading trace:', err);
      alert('Failed to load trace: ' + filename);
    });
}

function renderCodeView() {
  const container = document.getElementById('code-view');
  if (!container) {
    console.error('code-view container not found!');
    return;
  }

  if (!sourceMap || sourceMap.length === 0) {
    container.innerHTML = '<div style="color: #94a3b8; text-align: center; padding: 40px; background: rgba(148, 163, 184, 0.1); border-radius: 8px; border: 2px dashed #475569;"><p style="margin: 0; font-size: 1rem;">📝 No source code available for this trace.</p><p style="margin: 10px 0 0 0; font-size: 0.85rem; opacity: 0.7;">Source maps are generated during assembly.</p></div>';
    return;
  }

  let html = '';
  sourceMap.forEach((entry, idx) => {
    let bytesStr = '';
    if (entry.bytes && entry.bytes.length > 0) {
      bytesStr = entry.bytes.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
    }

    let addrStr = entry.address !== undefined ? '0x' + entry.address.toString(16).padStart(4, '0').toUpperCase() : '';

    html += `<div class="code-line" id="code-line-${idx}" data-addr="${entry.address}">
            <span class="line-num">${entry.line}</span>
            <span class="line-addr">${addrStr}</span>
            <span class="line-bytes">${bytesStr}</span>
            <span class="line-src">${escapeHtml(entry.source)}</span>
        </div>`;
  });
  container.innerHTML = html;
}

function escapeHtml(text) {
  if (!text) return '';
  return text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

// Playback control functions
function togglePlayback() {
  if (isPlaying) {
    stopPlayback();
  } else {
    startPlayback();
  }
}

function startPlayback() {
  isPlaying = true;
  const playBtn = document.getElementById('play-pause-btn');
  playBtn.textContent = '⏸️ Pause';
  playBtn.classList.add('running');

  if (playSpeed === 0) {
    // Max speed - use requestAnimationFrame
    const runMaxSpeed = () => {
      if (!isPlaying) return;
      if (currentCycle < trace.length - 1) {
        goToNext();
        requestAnimationFrame(runMaxSpeed);
      } else {
        stopPlayback();
      }
    };
    requestAnimationFrame(runMaxSpeed);
  } else {
    // Normal speed with interval
    playInterval = setInterval(() => {
      if (currentCycle < trace.length - 1) {
        goToNext();
      } else {
        stopPlayback();
      }
    }, playSpeed);
  }
}

function stopPlayback() {
  isPlaying = false;
  if (playInterval) {
    clearInterval(playInterval);
    playInterval = null;
  }
  const playBtn = document.getElementById('play-pause-btn');
  playBtn.textContent = '▶️ Run';
  playBtn.classList.remove('running');
}

function goToFirst() {
  currentCycle = 0;
  document.getElementById('slider').value = 0;
  render(0);
}

function goToPrevious() {
  if (currentCycle > 0) {
    currentCycle--;
    document.getElementById('slider').value = currentCycle;
    render(currentCycle);
  }
}

function goToNext() {
  if (currentCycle < trace.length - 1) {
    currentCycle++;
    document.getElementById('slider').value = currentCycle;
    render(currentCycle);
  }
}

function goToLast() {
  currentCycle = trace.length - 1;
  document.getElementById('slider').value = currentCycle;
  render(currentCycle);
}

function updateProgress() {
  if (trace.length === 0) return;
  const percent = ((currentCycle / (trace.length - 1)) * 100).toFixed(1);
  document.getElementById('progress-percent').textContent = `Progress: ${percent}%`;
}

const OPCODE_MNEMONICS = [
  "NOP", "HALT", "MOV", "LOAD", "STORE", "ADD", "SUB", "AND", "OR", "XOR",
  "CMP", "SHL", "SHR", "JMP", "JZ", "JNZ", "JC", "JNC", "JN", "CALL",
  "RET", "PUSH", "POP", "IN", "OUT"
];

const MODE_NAMES = [
  "REG", "IMM", "DIR", "IND", "OFF", "REL"
];

function render(idx) {
  if (!trace || !trace[idx]) return;
  const c = trace[idx];

  // Update cycle display
  document.getElementById('cycle').textContent = `Cycle: ${c.cycle} / ${trace.length - 1}`;
  updateProgress();

  // Registers
  let regsHtml = '<table>';
  for (let r in c.registers) {
    regsHtml += `<tr><td>${r}</td><td>${formatValue(c.registers[r], 4)}</td></tr>`;
  }

  // Flags
  regsHtml += `<tr><td>FLAGS</td><td>${renderFlags(c.flags)}</td></tr>`;

  // System / internal registers (if present)
  if (c.sp !== undefined) {
    regsHtml += `<tr><td>SP</td><td>${formatValue(c.sp, 4)}</td></tr>`;
  }
  if (c.ir !== undefined) regsHtml += `<tr><td>IR</td><td>${formatValue(c.ir, 4)}</td></tr>`;
  if (c.mar !== undefined) regsHtml += `<tr><td>MAR</td><td>${formatValue(c.mar, 4)}</td></tr>`;
  if (c.mdr !== undefined) regsHtml += `<tr><td>MDR</td><td>${formatValue(c.mdr, 4)}</td></tr>`;

  regsHtml += `</table>`;
  document.getElementById('registers').innerHTML = regsHtml;

  // Instruction
  let instrHtml = '';
  if (c.instr) {
    let opcodeNum = parseInt(c.instr.opcode, 10);
    let modeNum = parseInt(c.instr.mode, 10);
    let mnemonic = (opcodeNum >= 0 && opcodeNum < OPCODE_MNEMONICS.length) ? OPCODE_MNEMONICS[opcodeNum] : "?";
    let modeName = (modeNum >= 0 && modeNum < MODE_NAMES.length) ? MODE_NAMES[modeNum] : "?";

    instrHtml += `<pre>PC:     ${formatValue(c.pc, 4)}
Opcode: ${mnemonic} (${opcodeNum})
Mode:   ${modeName} (${modeNum})
Rd:     ${c.instr.rd}
Rs:     ${c.instr.rs}
Extra:  ${formatValue(c.instr.extra, 4)}</pre>`;
  } else {
    instrHtml += `<pre>PC: ${formatValue(c.pc, 4)}

(No instruction data available)</pre>`;
  }
  document.getElementById('instr').innerHTML = instrHtml;

  // Memory Layout Visualization
  renderMemoryLayout(c);

  // Memory Operations - Show current cycle writes
  let memHtml = '';
  if (c.mem_writes && c.mem_writes.length) {
    memHtml += '<ul>';
    c.mem_writes.forEach(m => memHtml += `<li>Addr ${formatValue(m.addr, 4)}: ${formatValue(m.old, 2)} → ${formatValue(m.new, 2)}</li>`);
    memHtml += '</ul>';
  } else {
    memHtml += '<p style="color: var(--text-secondary); font-style: italic;">No memory writes this cycle</p>';
  }

  // Add summary of all memory writes up to this point
  const allWrites = getAllMemoryWrites(currentCycle);
  if (allWrites.size > 0) {
    memHtml += '<div style="margin-top: 15px; padding-top: 15px; border-top: 2px solid var(--border-color);">';
    memHtml += '<strong>Cumulative Memory State (All Writes):</strong>';
    memHtml += '<ul style="max-height: 150px; overflow-y: auto;">';
    const sortedAddrs = Array.from(allWrites.keys()).sort((a, b) => a - b);
    sortedAddrs.forEach(addr => {
      const value = allWrites.get(addr);
      memHtml += `<li>${formatValue(addr, 4)}: ${formatValue(value, 2)}</li>`;
    });
    memHtml += '</ul></div>';
  }

  document.getElementById('mem').innerHTML = memHtml;

  // Stack Memory Visualization
  renderStackMemory(c);


  // Highlight Code
  if (sourceMap.length > 0) {
    // Parse PC from trace (format "0x1234")
    let pcVal = 0;
    if (typeof c.pc === 'string') pcVal = parseInt(c.pc, 16);
    else pcVal = c.pc;

    // Remove active class
    document.querySelectorAll('.code-line.active').forEach(el => el.classList.remove('active'));

    // Find lines with this address
    const lines = document.querySelectorAll(`.code-line[data-addr="${pcVal}"]`);
    if (lines.length > 0) {
      // Prefer the last one (usually the instruction, after labels)
      const el = lines[lines.length - 1];
      el.classList.add('active');

      // Scroll within the code-view container only, not the entire page
      const codeView = document.getElementById('code-view');
      if (codeView) {
        const elTop = el.offsetTop;
        const elHeight = el.offsetHeight;
        const containerHeight = codeView.clientHeight;
        const scrollTop = codeView.scrollTop;

        // Only scroll if element is not visible in the container
        if (elTop < scrollTop || elTop + elHeight > scrollTop + containerHeight) {
          codeView.scrollTop = elTop - (containerHeight / 2) + (elHeight / 2);
        }
      }
    }
  }
}

function toHexNumber(v, width = 4) {
  let n = Number(v) & 0xFFFF;
  return '0x' + n.toString(16).padStart(width, '0').toUpperCase();
}

function formatValue(val, width = 4) {
  if (val === undefined || val === null) return '-';
  if (typeof val === 'string') {
    if (val.startsWith('0x') || val.startsWith('0X')) return val.toUpperCase();
    let n = Number(val);
    if (!Number.isNaN(n)) return toHexNumber(n, width);
    return val;
  }
  if (typeof val === 'number') return toHexNumber(val, width);
  return String(val);
}

function renderFlags(flags) {
  let num = 0;
  if (typeof flags === 'string') {
    if (flags.startsWith('0x') || flags.startsWith('0X')) num = parseInt(flags, 16);
    else num = parseInt(flags, 10);
  } else if (typeof flags === 'number') {
    num = flags;
  }
  num = num & 0x0F;
  let names = ['Z', 'N', 'C', 'V'];
  let out = '';
  for (let i = 0; i < names.length; ++i) {
    let bit = (num >> (3 - i)) & 1;
    let color = bit ? 'var(--success-color)' : 'var(--text-secondary)';
    let weight = bit ? 'bold' : 'normal';
    out += `<span style="color:${color}; font-weight:${weight}; margin-right:8px; font-size:1.1em">${names[i]}</span>`;
  }
  return out;
}

// Helper function to get all memory writes up to a given cycle
function getAllMemoryWrites(upToCycle) {
  const memoryMap = new Map();
  for (let i = 0; i <= upToCycle && i < trace.length; i++) {
    if (trace[i] && trace[i].mem_writes) {
      trace[i].mem_writes.forEach(write => {
        const addr = typeof write.addr === 'string' ? parseInt(write.addr, 16) : write.addr;
        const val = typeof write.new === 'string' ? parseInt(write.new, 16) : write.new;
        memoryMap.set(addr, val);
      });
    }
  }
  return memoryMap;
}

// Memory Layout Visualization
function renderMemoryLayout(c) {
  const layoutView = document.getElementById('memory-layout');
  if (!layoutView) return;

  // Get current PC and SP
  let pcVal = 0x8000;
  let spVal = 0x7FFF;

  if (c.pc !== undefined) {
    if (typeof c.pc === 'string') pcVal = parseInt(c.pc, 16);
    else pcVal = c.pc;
  }

  if (c.sp !== undefined) {
    if (typeof c.sp === 'string') spVal = parseInt(c.sp, 16);
    else spVal = c.sp;
  }

  // Calculate stack usage
  const stackUsed = 0x7FFF - spVal;
  const stackDepth = Math.floor(stackUsed / 2); // Each stack entry is 2 bytes

  // Check if program is halted
  let isHalted = false;
  if (c.instr && c.instr.opcode !== undefined) {
    const opcode = typeof c.instr.opcode === 'string' ? parseInt(c.instr.opcode, 10) : c.instr.opcode;
    isHalted = (opcode === 1); // HALT opcode is 1
  }

  // Determine which segment is active
  const isInCode = pcVal >= 0x8000 && pcVal <= 0xEFFF && !isHalted;
  const isInData = pcVal >= 0x0000 && pcVal <= 0x0FFF;
  const isInStack = pcVal >= 0x1000 && pcVal <= 0x7FFF;

  let html = '';

  // Data Segment
  html += `<div class="memory-segment ${isInData ? 'active' : ''}">
    <h4>📦 Data Segment</h4>
    <div class="memory-segment-info">
      <strong>Range:</strong> 0x0000 - 0x0FFF<br>
      <strong>Size:</strong> 4 KiB<br>
      <strong>Purpose:</strong> Global variables, constants<br>
      <strong>Status:</strong> ${isInData ? '🟢 Active' : 'Inactive'}
    </div>
  </div>`;

  // Stack Segment
  html += `<div class="memory-segment ${isInStack ? 'active' : ''}">
    <h4>📚 Stack Segment</h4>
    <div class="memory-segment-info">
      <strong>Range:</strong> 0x1000 - 0x7FFF<br>
      <strong>Size:</strong> 28 KiB<br>
      <strong>Purpose:</strong> Function calls, recursion<br>
      <strong>SP:</strong> ${formatValue(spVal, 4)}<br>
      <strong>Stack Used:</strong> ${stackUsed} bytes (${stackDepth} entries)<br>
      <strong>Recursion Depth:</strong> ~${Math.max(0, stackDepth - 1)} levels
    </div>
  </div>`;

  // Code Segment
  const codeStatus = isHalted ? '🔴 Halted' : (isInCode ? '🟢 Executing' : '⚪ Inactive');
  html += `<div class="memory-segment ${isInCode ? 'active' : ''}">
    <h4>⚙️ Code Segment</h4>
    <div class="memory-segment-info">
      <strong>Range:</strong> 0x8000 - 0xEFFF<br>
      <strong>Size:</strong> 28 KiB<br>
      <strong>Purpose:</strong> Program instructions<br>
      <strong>PC:</strong> ${formatValue(pcVal, 4)}<br>
      <strong>Status:</strong> ${codeStatus}
    </div>
  </div>`;

  // Memory-Mapped I/O
  html += `<div class="memory-segment">
    <h4>🔌 Memory-Mapped I/O</h4>
    <div class="memory-segment-info">
      <strong>Range:</strong> 0xF000 - 0xF0FF<br>
      <strong>Size:</strong> 256 bytes<br>
      <strong>Purpose:</strong> I/O devices, timers
    </div>
  </div>`;

  layoutView.innerHTML = html;
}

// Stack Memory Visualization
function renderStackMemory(c) {
  const stackView = document.getElementById('stack-view');
  if (!stackView) return;

  // Get current SP value
  let spVal = 0x7FFF; // Default initial SP
  if (c.sp !== undefined) {
    if (typeof c.sp === 'string') spVal = parseInt(c.sp, 16);
    else spVal = c.sp;
  }

  // Stack grows downward from 0x7FFF
  const STACK_TOP = 0x7FFF;
  const WINDOW_SIZE = 100; // Increased to ensure deep stacks are rendered

  let html = '';

  // If SP is at initial position, show a message
  if (spVal === STACK_TOP) {
    html = '<div style="color: #94a3b8; text-align: center; padding: 20px; background: rgba(148, 163, 184, 0.1); border-radius: 6px;">📭 Stack is empty (SP = 0x7FFF)<br><small>Stack will grow downward as functions are called</small></div>';
  } else {
    // Calculate recursion depth
    const stackUsed = STACK_TOP - spVal;
    const depth = Math.floor(stackUsed / 6); // Approximate: each recursion level uses ~6 bytes

    html += `<div style="background: rgba(16, 185, 129, 0.1); padding: 8px; border-radius: 6px; margin-bottom: 10px; text-align: center;">
      <strong style="color: var(--success-color);">Stack Depth: ${depth} levels</strong><br>
      <small style="color: var(--text-secondary);">Stack used: ${stackUsed} bytes</small>
    </div>`;

    // Collect memory writes to build a memory state map
    const memoryMap = getAllMemoryWrites(currentCycle);

    // Display stack entries (word-aligned, 2 bytes per entry)
    html += '<div style="font-family: monospace; font-size: 0.75rem;">';
    let entryCount = 0;

    for (let addr = STACK_TOP; addr >= spVal - 10 && entryCount < WINDOW_SIZE; addr -= 2) {
      // Read 16-bit word from memory (big-endian)
      const highByte = memoryMap.get(addr) || 0;
      const lowByte = memoryMap.get(addr + 1) || 0;
      const word = (highByte << 8) | lowByte;

      const isSP = (addr === spVal);
      const isAboveSP = addr > spVal;
      const cssClass = isSP ? 'stack-entry sp-location' : 'stack-entry';

      let label = '';
      if (isSP) {
        label = '<span class="stack-label">← SP (Current Top)</span>';
      } else if (isAboveSP && word !== 0) {
        // Likely a return address or saved value
        if (word >= 0x8000 && word <= 0xEFFF) {
          label = '<span style="color: #a5b4fc; font-size: 0.7rem;">← Return Addr?</span>';
        } else {
          label = '<span style="color: #94a3b8; font-size: 0.7rem;">← Saved Value</span>';
        }
      }

      // Only show non-zero entries or entries near SP
      if (word !== 0 || Math.abs(addr - spVal) <= 10) {
        html += `<div class="${cssClass}" id="${isSP ? 'sp-marker' : ''}">
                  <span class="stack-addr">${formatValue(addr, 4)}</span>
                  <span class="stack-value">${formatValue(word, 4)}</span>
                  ${label}
                </div>`;
        entryCount++;
      }
    }

    html += '</div>';

    if (entryCount === 0) {
      html = '<div style="color: #94a3b8; text-align: center; padding: 20px;">No stack data to display</div>';
    }
  }

  stackView.innerHTML = html;

  // Auto-scroll to SP
  const spElement = stackView.querySelector('.sp-location');
  if (spElement) {
    spElement.scrollIntoView({ behavior: 'smooth', block: 'center' });
  }
}