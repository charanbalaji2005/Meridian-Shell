import React from 'react';
import { Search, Moon, Sun, Menu, X } from 'lucide-react';

interface HeaderProps {
  onOpenSearch: () => void;
  onToggleSidebar: () => void;
  isSidebarOpen: boolean;
  theme: 'dark' | 'light';
  onToggleTheme: () => void;
  currentSectionTitle: string;
}

export const Header: React.FC<HeaderProps> = ({
  onOpenSearch,
  onToggleSidebar,
  isSidebarOpen,
  theme,
  onToggleTheme,
  currentSectionTitle,
}) => {
  return (
    <header className="meridian-header">
      <div className="header-left">
        <button
          className="mobile-menu-btn"
          onClick={onToggleSidebar}
          aria-label="Toggle navigation"
        >
          {isSidebarOpen ? <X size={20} /> : <Menu size={20} />}
        </button>

        <a href="#intro" className="brand-logo" title="Meridian Shell Documentation">
          <div className="logo-badge">
            <svg viewBox="0 0 512 512" width="22" height="22" className="logo-svg">
              <defs>
                <linearGradient id="navGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                  <stop offset="0%" stopColor="#00E5FF" />
                  <stop offset="50%" stopColor="#3B82F6" />
                  <stop offset="100%" stopColor="#A855F7" />
                </linearGradient>
                <linearGradient id="navRing" x1="0%" y1="100%" x2="100%" y2="0%">
                  <stop offset="0%" stopColor="#A855F7" stopOpacity="0.4" />
                  <stop offset="50%" stopColor="#00E5FF" stopOpacity="0.9" />
                  <stop offset="100%" stopColor="#3B82F6" stopOpacity="0.5" />
                </linearGradient>
              </defs>
              <ellipse cx="256" cy="256" rx="210" ry="95" fill="none" stroke="url(#navRing)" strokeWidth="20" strokeDasharray="20 15" transform="rotate(-28 256 256)" />
              <g fill="none" stroke="url(#navGrad)" strokeWidth="48" strokeLinecap="round" strokeLinejoin="round">
                <polyline points="100,360 100,160 256,290 412,160 412,360" />
              </g>
              <polygon points="420,92 425,105 438,110 425,115 420,128 415,115 402,110 415,105" fill="#00E5FF" />
            </svg>
          </div>
          <div className="brand-text-block">
            <span className="brand-name">MERIDIAN SHELL</span>
            <span className="brand-pipe">|</span>
            <span className="brand-tag">DOCS</span>
          </div>
        </a>

        <div className="brand-subnav">
          <span className="brand-section-label">{currentSectionTitle}</span>
        </div>
      </div>

      <div className="header-center">
        <div className="search-box-wrapper" onClick={onOpenSearch}>
          <Search size={15} className="search-icon" />
          <span className="search-placeholder">Search documentation...</span>
          <span className="search-kbd">/</span>
        </div>
      </div>

      <div className="header-right">
        {/* Language selector */}
        <div className="header-select-wrapper">
          <select aria-label="Language selector" className="header-select" defaultValue="en-US">
            <option value="en-US">en-US</option>
          </select>
        </div>

        {/* Version selector */}
        <div className="header-select-wrapper">
          <select aria-label="Version selector" className="header-select" defaultValue="v2.0">
            <option value="v2.0">v2.0 (Latest)</option>
            <option value="main">main (git)</option>
          </select>
        </div>

        {/* GitHub link */}
        <a
          href="https://github.com/charanbalaji2005/Meridian-Shell"
          target="_blank"
          rel="noreferrer"
          className="header-icon-btn"
          title="GitHub Repository"
        >
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <path d="M15 22v-4a4.8 4.8 0 0 0-1-3.5c3 0 6-2 6-5.5.08-1.25-.27-2.48-1-3.5.28-1.15.28-2.35 0-3.5 0 0-1 0-3 1.5-2.64-.5-5.36-.5-8 0C6 2 5 2 5 2c-.3 1.15-.3 2.35 0 3.5A5.403 5.403 0 0 0 4 9c0 3.5 3 5.5 6 5.5-.39.49-.68 1.05-.85 1.65-.17.6-.22 1.23-.15 1.85v4" />
            <path d="M9 18c-4.51 2-5-2-7-2" />
          </svg>
        </a>

        {/* Theme Toggle */}
        <button
          onClick={onToggleTheme}
          className="header-icon-btn theme-toggle-btn"
          title={`Switch to ${theme === 'dark' ? 'light' : 'dark'} mode`}
        >
          {theme === 'dark' ? <Moon size={16} /> : <Sun size={16} />}
        </button>
      </div>
    </header>
  );
};
