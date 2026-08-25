import React from 'react';
import { Search, Moon, Sun, Edit3, Menu, X, Terminal } from 'lucide-react';

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
    <header className="fedora-header">
      <div className="header-left">
        <button
          className="mobile-menu-btn"
          onClick={onToggleSidebar}
          aria-label="Toggle navigation"
        >
          {isSidebarOpen ? <X size={20} /> : <Menu size={20} />}
        </button>

        <a href="#intro" className="brand-logo" title="Meridian Documentation">
          <div className="logo-badge">
            <svg viewBox="0 0 256 256" width="20" height="20" className="logo-svg">
              <g fill="none" stroke="currentColor" strokeWidth="24" strokeLinecap="round" strokeLinejoin="round">
                <polyline points="44,72 108,128 44,184" />
                <path d="M 84 24 A 64 104 0 0 1 84 232" />
                <line x1="172" y1="184" x2="212" y2="184" />
              </g>
            </svg>
          </div>
          <div className="brand-text">
            <span className="brand-title">DOCS</span>
          </div>
        </a>

        <div className="brand-subnav">
          <span className="brand-product">Meridian Docs</span>
          <span className="brand-divider">/</span>
          <span className="brand-section">{currentSectionTitle}</span>
        </div>
      </div>

      <div className="header-right">
        {/* Search Bar matching Fedora Docs design */}
        <div className="search-box-wrapper" onClick={onOpenSearch}>
          <Search size={15} className="search-icon" />
          <span className="search-placeholder">Search Docs</span>
          <span className="search-kbd">/</span>
          <label className="search-section-filter" onClick={(e) => e.stopPropagation()}>
            <input type="checkbox" defaultChecked />
            <span>In this section</span>
          </label>
        </div>

        {/* Language dropdown */}
        <div className="header-select-wrapper">
          <select aria-label="Language selector" className="header-select" defaultValue="en-US">
            <option value="en-US">en-US</option>
          </select>
        </div>

        {/* Version dropdown */}
        <div className="header-select-wrapper">
          <select aria-label="Version selector" className="header-select" defaultValue="f44">
            <option value="f44">f44 (v2.0)</option>
            <option value="main">main (git)</option>
          </select>
        </div>

        {/* GitHub link / Report Issue */}
        <a
          href="https://github.com/charanbalaji2005/Meridian-Shell/issues/new"
          target="_blank"
          rel="noreferrer"
          className="header-icon-btn"
          title="Report an issue or suggest edit"
        >
          <Edit3 size={16} />
        </a>

        <a
          href="https://github.com/charanbalaji2005/Meridian-Shell"
          target="_blank"
          rel="noreferrer"
          className="header-icon-btn"
          title="View GitHub Repository"
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
